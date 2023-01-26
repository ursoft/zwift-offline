#!/usr/bin/env python

import calendar
import datetime
import logging
import os
import signal
import platform
import random
import sys
import tempfile
import time
import math
import threading
import re
import smtplib, ssl
import requests
import json
import base64
import uuid
import jwt
import sqlalchemy
import xml.etree.ElementTree as ET
from copy import deepcopy
from functools import wraps
from io import BytesIO
from shutil import copyfile
from logging.handlers import RotatingFileHandler
from urllib.parse import quote
from flask import Flask, request, jsonify, redirect, render_template, url_for, flash, session, abort, make_response, send_file, send_from_directory
from flask_login import UserMixin, AnonymousUserMixin, LoginManager, login_user, current_user, login_required, logout_user
from gevent.pywsgi import WSGIServer
from google.protobuf.json_format import MessageToDict, Parse
from flask_sqlalchemy import SQLAlchemy
from werkzeug.security import generate_password_hash, check_password_hash
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes

sys.path.append(os.path.join(sys.path[0], 'protobuf')) # otherwise import in .proto does not work
import udp_node_msgs_pb2
import tcp_node_msgs_pb2
import activity_pb2
import goal_pb2
import login_pb2
import per_session_info_pb2
import profile_pb2
import segment_result_pb2
import world_pb2
import zfiles_pb2
import hash_seeds_pb2
import events_pb2
import variants_pb2
import playback_pb2
import online_sync

logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"))
logger = logging.getLogger('zoffline')
logger.setLevel(logging.DEBUG)
logging.getLogger('sqlalchemy.engine').setLevel(logging.WARN)

if getattr(sys, 'frozen', False):
    # If we're running as a pyinstaller bundle
    SCRIPT_DIR = sys._MEIPASS
    STORAGE_DIR = "%s/storage" % os.path.dirname(sys.executable)
    LOGS_DIR = "%s/logs" % os.path.dirname(sys.executable)
else:
    SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
    STORAGE_DIR = "%s/storage" % SCRIPT_DIR
    LOGS_DIR = "%s/logs" % SCRIPT_DIR

try:
    # Ensure storage dir exists
    if not os.path.isdir(STORAGE_DIR):
        os.makedirs(STORAGE_DIR)
except IOError as e:
    logger.error("failed to create storage dir (%s):  %s", STORAGE_DIR, str(e))
    sys.exit(1)

SSL_DIR = "%s/ssl" % SCRIPT_DIR
DATABASE_PATH = "%s/zwift-offline.db" % STORAGE_DIR
DATABASE_CUR_VER = 3

PACE_PARTNERS_DIR = "%s/pace_partners" % SCRIPT_DIR

# For auth server
AUTOLAUNCH_FILE = "%s/auto_launch.txt" % STORAGE_DIR
SERVER_IP_FILE = "%s/server-ip.txt" % STORAGE_DIR
if os.path.exists(SERVER_IP_FILE):
    with open(SERVER_IP_FILE, 'r') as f:
        server_ip = f.read().rstrip('\r\n')
else:
    server_ip = '127.0.0.1'
SECRET_KEY_FILE = "%s/secret-key.txt" % STORAGE_DIR
ENABLEGHOSTS_FILE = "%s/enable_ghosts.txt" % STORAGE_DIR
NEWHOME_FILE = "%s/new_home.txt" % STORAGE_DIR
MULTIPLAYER = os.path.exists("%s/multiplayer.txt" % STORAGE_DIR)
if MULTIPLAYER:
    try:
        if not os.path.isdir(LOGS_DIR):
            os.makedirs(LOGS_DIR)
    except IOError as e:
        logger.error("failed to create logs dir (%s):  %s", LOGS_DIR, str(e))
        sys.exit(1)
    from logging.handlers import RotatingFileHandler
    logHandler = RotatingFileHandler('%s/zoffline.log' % LOGS_DIR, maxBytes=1000000, backupCount=10)
    logger.addHandler(logHandler)
    CREDENTIALS_KEY_FILE = "%s/credentials-key.bin" % STORAGE_DIR
    if not os.path.exists(CREDENTIALS_KEY_FILE):
        with open(CREDENTIALS_KEY_FILE, 'wb') as f:
            f.write(get_random_bytes(32))
    with open(CREDENTIALS_KEY_FILE, 'rb') as f:
        credentials_key = f.read()

STRAVA_CLIENT_ID = '28117'
STRAVA_CLIENT_SECRET = '41b7b7b76d8cfc5dc12ad5f020adfea17da35468'

from tokens import *

# Android uses https for cdn
app = Flask(__name__, static_folder='%s/cdn/gameassets' % SCRIPT_DIR, static_url_path='/gameassets', template_folder='%s/cdn/static/web/launcher' % SCRIPT_DIR)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///{db}'.format(db=DATABASE_PATH)
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
if not os.path.exists(SECRET_KEY_FILE):
    with open(SECRET_KEY_FILE, 'wb') as f:
        f.write(os.urandom(16))
with open(SECRET_KEY_FILE, 'rb') as f:
    app.config['SECRET_KEY'] = f.read()
app.config['MAX_CONTENT_LENGTH'] = 1024 * 1024
db = SQLAlchemy()
db.init_app(app)

online = {}
global_pace_partners = {}
global_bots = {}
global_ghosts = {}
ghosts_enabled = {}
player_update_queue = {}
zc_connect_queue = {}
player_partial_profiles = {}
restarting = False
restarting_in_minutes = 0
reload_pacer_bots = False

class User(UserMixin, db.Model):
    player_id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(100), unique=True, nullable=False)
    first_name = db.Column(db.String(100), nullable=False)
    last_name = db.Column(db.String(100), nullable=False)
    pass_hash = db.Column(db.String(100), nullable=False)
    enable_ghosts = db.Column(db.Integer, nullable=False, default=1)
    new_home = db.Column(db.Integer, nullable=False, default=0)
    is_admin = db.Column(db.Integer, nullable=False, default=0)
    remember = db.Column(db.Integer, nullable=False, default=0)

    def __repr__(self):
        return self.username

    def get_id(self):
        return self.player_id

    def get_token(self):
        dt = datetime.datetime.utcnow() + datetime.timedelta(minutes=30)
        return jwt.encode({'user': self.player_id, 'exp': dt}, app.config['SECRET_KEY'], algorithm='HS256')

    @staticmethod
    def verify_token(token):
        try:
            data = jwt.decode(token, app.config['SECRET_KEY'], algorithms='HS256')
        except:
            return None
        id = data.get('user')
        if id:
            return User.query.get(id)
        return None

class AnonUser(User, AnonymousUserMixin, db.Model):
    username = "zoffline"
    first_name = "z"
    last_name = "offline"
    enable_ghosts = True
    new_home = False

    def is_authenticated(self):
        return True

class Activity(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    player_id = db.Column(db.Integer)
    course_id = db.Column(db.Integer)
    name = db.Column(db.Text)
    f5 = db.Column(db.Integer)
    privateActivity = db.Column(db.Integer)
    start_date = db.Column(db.Text)
    end_date = db.Column(db.Text)
    distanceInMeters = db.Column(db.Float)
    avg_heart_rate = db.Column(db.Float)
    max_heart_rate = db.Column(db.Float)
    avg_watts = db.Column(db.Float)
    max_watts = db.Column(db.Float)
    avg_cadence = db.Column(db.Float)
    max_cadence = db.Column(db.Float)
    avg_speed = db.Column(db.Float)
    max_speed = db.Column(db.Float)
    calories = db.Column(db.Float)
    total_elevation = db.Column(db.Float)
    strava_upload_id = db.Column(db.Integer)
    strava_activity_id = db.Column(db.Integer)
    f22 = db.Column(db.Text)
    f23 = db.Column(db.Integer)
    fit = db.Column(db.LargeBinary)
    fit_filename = db.Column(db.Text)
    subgroupId = db.Column(db.Integer)
    workoutHash = db.Column(db.Integer)
    progressPercentage = db.Column(db.Float)
    sport = db.Column(db.Integer)
    date = db.Column(db.Text)
    act_f32 = db.Column(db.Float)
    act_f33 = db.Column(db.Text)
    act_f34 = db.Column(db.Text)
    privacy = db.Column(db.Integer)
    fitness_privacy = db.Column(db.Integer)
    club_name = db.Column(db.Text)
    movingTimeInMs = db.Column(db.Integer)

class SegmentResult(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    player_id = db.Column(db.Integer)
    server_realm = db.Column(db.Integer)
    course_id = db.Column(db.Integer)
    segment_id = db.Column(db.Integer)
    event_subgroup_id = db.Column(db.Integer)
    first_name = db.Column(db.Text)
    last_name = db.Column(db.Text)
    world_time = db.Column(db.Integer)
    finish_time_str = db.Column(db.Text)
    elapsed_ms = db.Column(db.Integer)
    power_source_model = db.Column(db.Integer)
    weight_in_grams = db.Column(db.Integer)
    f14 = db.Column(db.Integer)
    avg_power = db.Column(db.Integer)
    is_male = db.Column(db.Integer)
    time = db.Column(db.Text)
    player_type = db.Column(db.Integer)
    avg_hr = db.Column(db.Integer)
    sport = db.Column(db.Integer)
    activity_id = db.Column(db.Integer)
    f22 = db.Column(db.Integer)
    f23 = db.Column(db.Text)

class Goal(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    player_id = db.Column(db.Integer)
    sport = db.Column(db.Integer)
    name = db.Column(db.Text)
    type = db.Column(db.Integer)
    periodicity = db.Column(db.Integer)
    target_distance = db.Column(db.Float)
    target_duration = db.Column(db.Float)
    actual_distance = db.Column(db.Float)
    actual_duration = db.Column(db.Float)
    created_on = db.Column(db.Integer)
    period_end_date = db.Column(db.Integer)
    status = db.Column(db.Integer)
    timezone = db.Column(db.Text)

class Playback(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    player_id = db.Column(db.Integer, nullable=False)
    uuid = db.Column(db.Text, nullable=False)
    segment_id = db.Column(db.Integer, nullable=False)
    time = db.Column(db.Float, nullable=False)
    world_time = db.Column(db.Integer, nullable=False)

class Zfile(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    folder = db.Column(db.Text, nullable=False)
    filename = db.Column(db.Text, nullable=False)
    timestamp = db.Column(db.Integer, nullable=False)
    player_id = db.Column(db.Integer, nullable=False)

class PrivateEvent(db.Model): # cached in glb_private_events
    id = db.Column(db.Integer, primary_key=True)
    json = db.Column(db.Text, nullable=False)

class Notification(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    event_id = db.Column(db.Integer, nullable=False)
    player_id = db.Column(db.Integer, nullable=False)
    json = db.Column(db.Text, nullable=False)

class Version(db.Model):
    version = db.Column(db.Integer, primary_key=True)

class Relay:
    def __init__(self, key = b''):
        self.ri = 0
        self.tcp_ci = 0
        self.udp_ci = 0
        self.tcp_r_sn = 0
        self.tcp_t_sn = 0
        self.udp_r_sn = 0
        self.udp_t_sn = 0
        self.key = key

class PartialProfile:
    player_id = 0
    first_name = ''
    last_name = ''
    country_code = 0
    route = 0
    player_type = 'NORMAL'
    male = True
    imageSrc = ''
    def to_json(self):
        return {"countryCode": self.country_code,
                "enrolledZwiftAcademy": False, #don't need
                "firstName": self.first_name,
                "id": self.player_id,
                "imageSrc": self.imageSrc,
                "lastName": self.last_name,
                "male": self.male,
                "playerType": self.player_type }

class Online:
    total = 0
    richmond = 0
    watopia = 0
    london = 0
    makuriislands = 0
    newyork = 0
    innsbruck = 0
    yorkshire = 0
    france = 0
    paris = 0

courses_lookup = {
    2: 'Richmond',
    4: 'Unknown',  # event specific?
    6: 'Watopia',
    7: 'London',
    8: 'New York',
    9: 'Innsbruck',
    10: 'Bologna',  # event specific
    11: 'Yorkshire',
    12: 'Crit City',  # event specific
    13: 'Makuri Islands',
    14: 'France',
    15: 'Paris',
    16: 'Gravel Mountain'  # event specific
}

tree = ET.parse('%s/cdn/gameassets/GameDictionary.xml' % SCRIPT_DIR)
GD = tree.getroot()


def get_utc_time():
    return datetime.datetime.utcnow().timestamp()

def get_time():
    return datetime.datetime.now().timestamp()


def get_online():
    online_in_region = Online()
    for p_id in online:
        player_state = online[p_id]
        course = get_course(player_state)
        course_name = courses_lookup[course]
        if course_name == 'Richmond':
            online_in_region.richmond += 1
        elif course_name == 'Watopia':
            online_in_region.watopia += 1
        elif course_name == 'London':
            online_in_region.london += 1
        elif course_name == 'Makuri Islands':
            online_in_region.makuriislands += 1
        elif course_name == 'New York':
            online_in_region.newyork += 1
        elif course_name == 'Innsbruck':
            online_in_region.innsbruck += 1
        elif course_name == 'Yorkshire':
            online_in_region.yorkshire += 1
        elif course_name == 'France':
            online_in_region.france += 1
        elif course_name == 'Paris':
            online_in_region.paris += 1
        online_in_region.total += 1
    return online_in_region


def toSigned(n, byte_count):
    return int.from_bytes(n.to_bytes(byte_count, 'little'), 'little', signed=True)

def imageSrc(player_id):
    if os.path.isfile(os.path.join(STORAGE_DIR, str(player_id), 'avatarLarge.jpg')):
        return "https://us-or-rly101.zwift.com/download/%s/avatarLarge.jpg" % player_id
    else:
        return None

def get_partial_profile(player_id):
    if not player_id in player_partial_profiles:
        partial_profile = PartialProfile()
        partial_profile.player_id = player_id
        if player_id in global_pace_partners.keys():
            profile = global_pace_partners[player_id].profile
        elif player_id in global_bots.keys():
            profile = global_bots[player_id].profile
        else:
            #Read from disk
            profile_file = '%s/%s/profile.bin' % (STORAGE_DIR, player_id)
            if os.path.isfile(profile_file):
                with open(profile_file, 'rb') as fd:
                    profile = profile_pb2.PlayerProfile()
                    profile.ParseFromString(fd.read())
            else:
                user = User.query.filter_by(player_id=player_id).first()
                partial_profile.first_name = user.first_name
                partial_profile.last_name = user.last_name
                return partial_profile
        partial_profile.imageSrc = imageSrc(player_id)
        partial_profile.first_name = profile.first_name
        partial_profile.last_name = profile.last_name
        partial_profile.country_code = profile.country_code
        partial_profile.player_type = profile_pb2.PlayerType.Name(jsf(profile, 'player_type', 1))
        partial_profile.male = profile.is_male
        for f in profile.public_attributes:
            #0x69520F20=1766985504 - crc32 of "PACE PARTNER - ROUTE"
            #TODO: -1021012238: figure out
            if f.id == 1766985504 or f.id == -1021012238:  #-1021012238 == 3273955058
                if f.number_value >= 0:
                    partial_profile.route = toSigned(f.number_value, 4)
                else:
                    partial_profile.route = -toSigned(-f.number_value, 4)
                break
        player_partial_profiles[player_id] = partial_profile
    return player_partial_profiles[player_id]


def get_course(state):
    return (state.f19 & 0xff0000) >> 16

def road_id(state):
    return (state.aux3 & 0xff00) >> 8

def is_forward(state):
    return (state.f19 & 4) != 0

def is_nearby(s1, s2):
    if s1 is None or s2 is None:
        return False
    if s1.watchingRiderId == s2.id or s2.watchingRiderId == s1.id:
        return True
    if get_course(s1) == get_course(s2):
        dist = math.sqrt((s2.x - s1.x)**2 + (s2.z - s1.z)**2 + (s2.y_altitude - s1.y_altitude)**2)
        if dist <= 100000 or road_id(s1) == road_id(s2):
            return True
    return False


# We store flask-login's cookie in the "fake" JWT that we give Zwift.
# Make it a cookie again to reuse flask-login on API calls.
def jwt_to_session_cookie(f):
    @wraps(f)
    def wrapper(*args, **kwargs):
        if not MULTIPLAYER:
            return f(*args, **kwargs)
        token = request.headers.get('Authorization')
        if token and not session.get('_user_id'):
            token = jwt.decode(token.split()[1], options=({'verify_signature': False, 'verify_aud': False}))
            request.cookies = request.cookies.copy()  # request.cookies is an immutable dict
            request.cookies['remember_token'] = token['session_cookie']
            login_manager._load_user()

        return f(*args, **kwargs)
    return wrapper


@app.route("/signup/", methods=["GET", "POST"])
def signup():
    if request.method == "POST":
        username = request.form['username']
        password = request.form['password']
        confirm_password = request.form['confirm_password']
        first_name = request.form['first_name']
        last_name = request.form['last_name']

        if not (username and password and confirm_password and first_name and last_name):
            flash("All fields are required.")
            return redirect(url_for('signup'))
        if not re.match(r"[^@]+@[^@]+\.[^@]+", username):
            flash("Username is not a valid e-mail address.")
            return redirect(url_for('signup'))
        if password != confirm_password:
            flash("Passwords did not match.")
            return redirect(url_for('signup'))

        hashed_pwd = generate_password_hash(password, 'sha256')

        new_user = User(username=username, pass_hash=hashed_pwd, first_name=first_name, last_name=last_name)
        db.session.add(new_user)

        try:
            db.session.commit()
        except sqlalchemy.exc.IntegrityError:
            flash("Username {u} is not available.".format(u=username))
            return redirect(url_for('signup'))

        flash("User account has been created.")
        return redirect(url_for("login"))

    return render_template("signup.html")


@app.route("/login/", methods=["GET", "POST"])
def login():
    if request.method == "POST":
        username = request.form['username']
        password = request.form['password']
        remember = bool(request.form.get('remember'))

        if not (username and password):
            flash("Username and password cannot be empty.")
            return redirect(url_for('login'))

        user = User.query.filter_by(username=username).first()

        if user and check_password_hash(user.pass_hash, password):
            login_user(user, remember=True)
            user.remember = remember
            db.session.commit()
            profile_dir = os.path.join(STORAGE_DIR, str(user.player_id))
            try:
                if not os.path.isdir(profile_dir):
                    os.makedirs(profile_dir)
            except IOError as e:
                logger.error("failed to create profile dir (%s):  %s", profile_dir, str(e))
                return '', 500
            return redirect(url_for("user_home", username=username, enable_ghosts=bool(user.enable_ghosts), new_home=bool(user.new_home), online=get_online()))
        else:
            flash("Invalid username or password.")

    if current_user.is_authenticated and current_user.remember:
        return redirect(url_for("user_home", username=current_user.username, enable_ghosts=bool(current_user.enable_ghosts), new_home=bool(current_user.new_home), online=get_online()))

    user = User.verify_token(request.args.get('token'))
    if user:
        login_user(user, remember=False)
        return redirect(url_for("reset", username=user.username))

    return render_template("login_form.html")


@app.route("/forgot/", methods=["GET", "POST"])
def forgot():
    if request.method == "POST":
        username = request.form['username']
        if not username:
            flash("Username cannot be empty.")
            return redirect(url_for('forgot'))
        if not re.match(r"[^@]+@[^@]+\.[^@]+", username):
            flash("Username is not a valid e-mail address.")
            return redirect(url_for('forgot'))

        user = User.query.filter_by(username=username).first()
        if user:
            try:
                with open('%s/gmail_credentials.txt' % STORAGE_DIR, 'r') as f:
                    sender_email = f.readline().rstrip('\r\n')
                    password = f.readline().rstrip('\r\n')
                    with smtplib.SMTP_SSL("smtp.gmail.com", 465, context=ssl.create_default_context()) as server:
                        server.login(sender_email, password)
                        message = MIMEMultipart()
                        message['From'] = sender_email
                        message['To'] = username
                        message['Subject'] = "Password reset"
                        content = "https://%s/login/?token=%s" % (server_ip, user.get_token())
                        message.attach(MIMEText(content, 'plain'))
                        server.sendmail(sender_email, username, message.as_string())
                        server.close()
                        flash("E-mail sent.")
            except Exception as exc:
                logger.warn('send e-mail: %s' % repr(exc))
                flash("Could not send e-mail.")
        else:
            flash("Invalid username.")

    return render_template("forgot.html")

@app.route("/api/push/fcm/<type>/<token>", methods=["POST", "DELETE"])
@app.route("/api/push/fcm/<type>/<token>/enables", methods=["PUT"])
def api_push_fcm_production(type, token):
    return '', 500

@app.route("/api/users/password-reset/", methods=["POST"])
@jwt_to_session_cookie
@login_required
def api_users_password_reset():
    password = request.form.get("password-new")
    confirm_password = request.form.get("password-confirm")
    if password != confirm_password:
        return 'passwords not match', 500
    hashed_pwd = generate_password_hash(password, 'sha256')
    current_user.pass_hash = hashed_pwd
    db.session.commit()
    return '', 200

@app.route("/reset/<username>/", methods=["GET", "POST"])
@login_required
def reset(username):
    if request.method == "POST":
        password = request.form['password']
        confirm_password = request.form['confirm_password']

        if not (password and confirm_password):
            flash("All fields are required.")
            return redirect(url_for('reset', username=current_user.username))
        if password != confirm_password:
            flash("Passwords did not match.")
            return redirect(url_for('reset', username=current_user.username))

        hashed_pwd = generate_password_hash(password, 'sha256')
        current_user.pass_hash = hashed_pwd
        db.session.commit()
        flash("Password changed.")

    return render_template("reset.html", username=current_user.username)


@app.route("/strava", methods=['GET'])
@login_required
def strava():
    try:
        from stravalib.client import Client
    except ImportError as exc:
        logger.warn('stravalib: %s' % repr(exc))
        flash("stravalib is not installed. Skipping Strava authorization attempt.")
        return redirect('/user/%s/' % current_user.username)
    client = Client()
    url = client.authorization_url(client_id=STRAVA_CLIENT_ID,
                                   redirect_uri='https://launcher.zwift.com/authorization',
                                   scope='activity:write')
    return redirect(url)


@app.route("/authorization", methods=["GET", "POST"])
@login_required
def authorization():
    from stravalib.client import Client
    try: 
        client = Client()
        code = request.args.get('code')
        token_response = client.exchange_code_for_token(client_id=STRAVA_CLIENT_ID, client_secret=STRAVA_CLIENT_SECRET, code=code)
        with open(os.path.join(STORAGE_DIR, str(current_user.player_id), 'strava_token.txt'), 'w') as f:
            f.write(STRAVA_CLIENT_ID + '\n');
            f.write(STRAVA_CLIENT_SECRET + '\n');
            f.write(token_response['access_token'] + '\n');
            f.write(token_response['refresh_token'] + '\n');
            f.write(str(token_response['expires_at']) + '\n');
        flash("Strava authorized. Go to \"Profile\" to remove authorization.")
    except Exception as exc:
        logger.warn('Strava: %s' % repr(exc))
        flash("Strava canceled.")
    flash("Please close this window and return to Zwift Launcher.")
    return render_template("strava.html", username=current_user.username)


@app.route("/profile/<username>/", methods=["GET", "POST"])
@login_required
def profile(username):
    if request.method == "POST":
        if request.form['username'] == "" or request.form['password'] == "":
            flash("Zwift credentials can't be empty.")
            return render_template("profile.html", username=current_user.username)

        username = request.form['username']
        password = request.form['password']
        profile_dir = os.path.join(STORAGE_DIR, str(current_user.player_id))
        session = requests.session()

        try:
            access_token, refresh_token = online_sync.login(session, username, password)
            try:
                profile = online_sync.query_player_profile(session, access_token)
                with open('%s/profile.bin' % SCRIPT_DIR, 'wb') as f:
                    f.write(profile)
                online_sync.logout(session, refresh_token)
                os.rename('%s/profile.bin' % SCRIPT_DIR, '%s/profile.bin' % profile_dir)
                flash("Zwift profile installed locally.")
            except Exception as exc:
                logger.warn('Zwift profile: %s' % repr(exc))
                flash("Error downloading profile.")
            if request.form.get("save_zwift", None) != None:
                try:
                    zwift_credentials = (username + '\n' + password).encode('UTF-8')
                    cipher_suite = AES.new(credentials_key, AES.MODE_CFB)
                    ciphered_text = cipher_suite.encrypt(zwift_credentials)
                    file_path = os.path.join(profile_dir, 'zwift_credentials.bin')
                    with open(file_path, 'wb') as fw:
                        fw.write(cipher_suite.iv)
                        fw.write(ciphered_text)
                    flash("Zwift credentials saved. Go to \"Profile\" to remove credentials.")
                except Exception as exc:
                    logger.warn('zwift_credentials: %s' % repr(exc))
                    flash("Error saving 'zwift_credentials.bin' file.")
        except Exception as exc:
            logger.warn('online_sync.login: %s' % repr(exc))
            flash("Invalid username or password.")
    return render_template("profile.html", username=current_user.username)


@app.route("/garmin/<username>/", methods=["GET", "POST"])
@login_required
def garmin(username):
    if request.method == "POST":
        if request.form['username'] == "" or request.form['password'] == "":
            flash("Garmin credentials can't be empty.")
            return render_template("garmin.html", username=current_user.username)

        username = request.form['username']
        password = request.form['password']

        try:
            garmin_credentials = (username + '\n' + password).encode('UTF-8')
            cipher_suite = AES.new(credentials_key, AES.MODE_CFB)
            ciphered_text = cipher_suite.encrypt(garmin_credentials)
            file_path = os.path.join(STORAGE_DIR, str(current_user.player_id), 'garmin_credentials.bin')
            with open(file_path, 'wb') as fw:
                fw.write(cipher_suite.iv)
                fw.write(ciphered_text)
            flash("Garmin credentials saved. Go to \"Profile\" to remove credentials.")
        except Exception as exc:
            logger.warn('garmin_credentials: %s' % repr(exc))
            flash("Error saving 'garmin_credentials.bin' file.")
    return render_template("garmin.html", username=current_user.username)


@app.route("/user/<username>/")
@login_required
def user_home(username):
    return render_template("user_home.html", username=current_user.username, enable_ghosts=bool(current_user.enable_ghosts), new_home=bool(current_user.new_home),
        online=get_online(), is_admin=current_user.is_admin, restarting=restarting, restarting_in_minutes=restarting_in_minutes, server_ip=os.path.exists(SERVER_IP_FILE))

def enqueue_player_update(player_id, wa_bytes):
    if not player_id in player_update_queue:
        player_update_queue[player_id] = list()
    player_update_queue[player_id].append(wa_bytes)

def send_message_to_all_online(message, sender='Server'):
    player_update = udp_node_msgs_pb2.WorldAttribute()
    player_update.server_realm = udp_node_msgs_pb2.ZofflineConstants.RealmID
    player_update.wa_type = udp_node_msgs_pb2.WA_TYPE.WAT_SPA
    player_update.world_time_born = world_time()
    player_update.world_time_expire = world_time() + 60000
    player_update.wa_f12 = 1
    player_update.timestamp = int(get_utc_time()*1000000)

    chat_message = tcp_node_msgs_pb2.SocialPlayerAction()
    chat_message.player_id = 0
    chat_message.to_player_id = 0
    chat_message.spa_type = tcp_node_msgs_pb2.SocialPlayerActionType.SOCIAL_TEXT_MESSAGE
    chat_message.firstName = sender
    chat_message.lastName = ''
    chat_message.message = message
    chat_message.countryCode = 0

    player_update.payload = chat_message.SerializeToString()
    player_update_s = player_update.SerializeToString()
    for receiving_player_id in online.keys():
        enqueue_player_update(receiving_player_id, player_update_s)


def send_restarting_message():
    global restarting
    global restarting_in_minutes
    while restarting:
        send_message_to_all_online('Restarting / Shutting down in %s minutes. Save your progress or continue riding until server is back online' % restarting_in_minutes)
        time.sleep(60)
        restarting_in_minutes -= 1
        if restarting and restarting_in_minutes == 0:
            message = 'See you later! Look for the back online message.'
            send_message_to_all_online(message)
            discord.send_message(message)
            time.sleep(6)
            os.kill(os.getpid(), signal.SIGINT)


@app.route("/restart")
@login_required
def restart_server():
    global restarting
    global restarting_in_minutes
    if bool(current_user.is_admin):
        restarting = True
        restarting_in_minutes = 10
        send_restarting_message_thread = threading.Thread(target=send_restarting_message)
        send_restarting_message_thread.start()
        discord.send_message('Restarting / Shutting down in %s minutes. Save your progress or continue riding until server is back online' % restarting_in_minutes)
    return redirect('/user/%s/' % current_user.username)


@app.route("/cancelrestart")
@login_required
def cancel_restart_server():
    global restarting
    global restarting_in_minutes
    if bool(current_user.is_admin):
        restarting = False
        restarting_in_minutes = 0
        message = 'Restart of the server has been cancelled. Ride on!'
        send_message_to_all_online(message)
        discord.send_message(message)
    return redirect('/user/%s/' % current_user.username)


@app.route("/reloadbots")
@login_required
def reload_bots():
    global reload_pacer_bots
    if bool(current_user.is_admin):
        reload_pacer_bots = True
    return redirect('/user/%s/' % current_user.username)


@app.route("/upload/<username>/", methods=["GET", "POST"])
@login_required
def upload(username):
    profile_dir = os.path.join(STORAGE_DIR, str(current_user.player_id))

    if request.method == 'POST':
        uploaded_file = request.files['file']
        if uploaded_file.filename in ['profile.bin', 'achievements.bin']:
            file_path = os.path.join(profile_dir, uploaded_file.filename)
            uploaded_file.save(file_path)
            flash("File %s uploaded." % uploaded_file.filename)
        else:
            flash("Invalid file name.")

    name = ''
    profile = None
    profile_file = os.path.join(profile_dir, 'profile.bin')
    if os.path.isfile(profile_file):
        stat = os.stat(profile_file)
        profile = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(stat.st_mtime))
        with open(profile_file, 'rb') as fd:
            p = profile_pb2.PlayerProfile()
            p.ParseFromString(fd.read())
            name = "%s %s" % (p.first_name, p.last_name)
    token = None
    token_file = os.path.join(profile_dir, 'strava_token.txt')
    if os.path.isfile(token_file):
        stat = os.stat(token_file)
        token = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(stat.st_mtime))
    garmin = None
    garmin_file = os.path.join(profile_dir, 'garmin_credentials.bin')
    if os.path.isfile(garmin_file):
        stat = os.stat(garmin_file)
        garmin = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(stat.st_mtime))
    zwift = None
    zwift_file = os.path.join(profile_dir, 'zwift_credentials.bin')
    if os.path.isfile(zwift_file):
        stat = os.stat(zwift_file)
        zwift = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(stat.st_mtime))

    return render_template("upload.html", username=current_user.username, profile=profile, name=name, token=token, garmin=garmin, zwift=zwift)


@app.route("/download/profile.bin", methods=["GET"])
@login_required
def download_profile():
    profile_file = os.path.join(STORAGE_DIR, str(current_user.player_id), 'profile.bin')
    if os.path.isfile(profile_file):
        return send_file(profile_file)

@app.route("/download/<int:player_id>/avatarLarge.jpg", methods=["GET"])
def download_avatarLarge(player_id):
    profile_file = os.path.join(STORAGE_DIR, str(player_id), 'avatarLarge.jpg')
    if os.path.isfile(profile_file):
        return send_file(profile_file, mimetype='image/jpeg')
    else:
        return '', 404

@app.route("/delete/<filename>", methods=["GET"])
@login_required
def delete(filename):
    player_id = current_user.player_id
    if filename not in ['profile.bin', 'strava_token.txt', 'garmin_credentials.bin', 'zwift_credentials.bin']:
        return '', 403
    profile_dir = os.path.join(STORAGE_DIR, str(player_id))
    delete_file = os.path.join(profile_dir, filename)
    if os.path.isfile(delete_file):
        os.remove("%s" % delete_file)
    return redirect(url_for('upload', username=current_user))


@app.route("/logout/<username>")
@login_required
def logout(username):
    session.clear()
    logout_user()
    flash("Successfully logged out.")
    return redirect(url_for('login'))


def insert_protobuf_into_db(table_name, msg, exclude_fields=[]):
    msg_dict = MessageToDict(msg, preserving_proto_field_name=True, use_integers_for_enums=True)
    for key in exclude_fields:
        del msg_dict[key]
    if 'id' in msg_dict:
        del msg_dict['id']
    row = table_name(**msg_dict)
    db.session.add(row)
    db.session.commit()
    return row.id


def update_protobuf_in_db(table_name, msg, id, exclude_fields=[]):
    msg_dict = MessageToDict(msg, preserving_proto_field_name=True, use_integers_for_enums=True)
    for key in exclude_fields:
        del msg_dict[key]
    table_name.query.filter_by(id=id).update(msg_dict)
    db.session.commit()


def row_to_protobuf(row, msg, exclude_fields=[]):
    for key in row.keys():
        if key in exclude_fields:
            continue
        if row[key] is None:
            continue
        setattr(msg, key, row[key])
    return msg


def world_time():
    return int((get_utc_time()-1414016075)*1000)

@app.route('/api/clubs/club/can-create', methods=['GET'])
def api_clubs_club_cancreate():
    return jsonify({"reason": "DISABLED", "result": False})

@app.route('/api/event-feed', methods=['GET']) #from=1646723199600&limit=25&sport=CYCLING
def api_eventfeed():
    eventCount = int(request.args.get('limit', 50))
    sport = request.args.get('sport', 'CYCLING')
    events = get_events(eventCount, sport)
    json_events = convert_events_to_json(events)
    json_data = []
    for e in json_events:
        json_data.append({"event": e})
    return jsonify({"data":json_data,"cursor":None})

@app.route('/api/campaign/profile/campaigns', methods=['GET'])
@app.route('/api/announcements/active', methods=['GET'])
@app.route('/api/recommendation/profile', methods=['GET'])
def api_empty_arrays():
    return jsonify([])

def activity_moving_time(activity):
    try:
        return (datetime.strptime(activity.end_date, '%y-%m-%dT%H:%M:%SZ') - datetime.strptime(activity.start_date, '%y-%m-%dT%H:%M:%SZ')).total_seconds() * 1000
    except:
        return 0

def activity_protobuf_to_json(activity):
    profile = get_partial_profile(activity.player_id)
    return {"id":activity.id,"profile":{"id":str(activity.player_id),"firstName":profile.first_name,"lastName":profile.last_name, \
    "imageSrc":profile.imageSrc,"approvalRequired":None}, \
    "worldId":activity.course_id,"name":activity.name,"sport":str_sport(activity.sport),"startDate":activity.start_date, \
    "endDate":activity.end_date,"distanceInMeters":activity.distanceInMeters, \
    "totalElevation":activity.total_elevation,"calories":activity.calories,"primaryImageUrl":"", \
    "feedImageThumbnailUrl":"", \
    "lastSaveDate":activity.date,"movingTimeInMs":activity_moving_time(activity), \
    "avgSpeedInMetersPerSecond":activity.avg_speed,"activityRideOnCount":0,"activityCommentCount":0,"privacy":"PUBLIC", \
    "eventId":None,"rideOnGiven":False,"id_str":str(activity.id)}

def select_activities_json(player_id, limit):
    ret = []
    if limit > 0:
        activities = activity_pb2.ActivityList()
        stmt = sqlalchemy.text("SELECT * FROM activity WHERE player_id = :p ORDER BY date DESC LIMIT :l")
        rows = db.session.execute(stmt, {"p": player_id, "l": limit})
        allow_empty_end_date = True
        for row in rows:
            activity = activities.activities.add()
            row_to_protobuf(row, activity, exclude_fields=['fit'])
            if activity.end_date == "":
                if allow_empty_end_date:
                    allow_empty_end_date = False
                else:
                    continue
            ret.append(activity_protobuf_to_json(activity))
    return ret

@app.route('/api/activity-feed/feed/', methods=['GET'])
@jwt_to_session_cookie
@login_required
def api_activity_feed():
    limit = int(request.args.get('limit'))
    feed_type = request.args.get('feedType')
    if feed_type == 'JUST_ME' or feed_type == 'PREVIEW': #what is the difference here?
        ret = select_activities_json(current_user.player_id, limit)
    else: # todo: FAVORITES, FOLLOWEES
        ret = []
    return jsonify(ret)

@app.route('/api/auth', methods=['GET'])
def api_auth():
    return {"realm": "zwift","launcher": "https://launcher.zwift.com/launcher","url": "https://secure.zwift.com/auth/"}

@app.route('/api/server', methods=['GET'])
def api_server():
    return {"build":"zwift_1.267.0","version":"1.267.0"}

@app.route('/api/servers', methods=['GET'])
def api_servers():
    return {"baseUrl":"https://us-or-rly101.zwift.com/relay"}

@app.route('/api/clubs/club/list/my-clubs', methods=['GET'])
@app.route('/api/clubs/club/featured', methods=['GET'])
@app.route('/api/clubs/club', methods=['GET'])
def api_clubs():
    return jsonify({"total": 0, "results": []})

@app.route('/api/clubs/club/my-clubs-summary', methods=['GET'])
def api_clubs_club_my_clubs_summary():
    return jsonify({"invitedCount": 0, "requestedCount": 0, "results": []})

@app.route('/api/clubs/club/list/my-clubs.proto', methods=['GET'])
@app.route('/api/campaign/proto/campaigns', methods=['GET'])
def api_proto_empty():
    return '', 200

@app.route('/api/game_info/version', methods=['GET'])
def api_gameinfo_version():
    game_info_file = os.path.join(SCRIPT_DIR, "game_info.txt")
    with open(game_info_file, mode="r", encoding="utf-8-sig") as f:
        data = json.load(f)
        return {"version": data['gameInfoHash']}

@app.route('/api/game_info', methods=['GET'])
def api_gameinfo():
    game_info_file = os.path.join(SCRIPT_DIR, "game_info.txt")
    with open(game_info_file, mode="r", encoding="utf-8-sig") as f:
        r = make_response(f.read())
        r.mimetype = 'application/json'
        return r

@app.route('/api/users/login', methods=['POST'])
@jwt_to_session_cookie
@login_required
def api_users_login():
    req = login_pb2.LoginRequest()
    req.ParseFromString(request.stream.read())
    player_id = current_user.player_id
    global_relay[player_id] = Relay(req.key)

    response = login_pb2.LoginResponse()
    response.session_state = 'abc'
    response.info.relay_url = "https://us-or-rly101.zwift.com/relay"
    response.info.apis.todaysplan_url = "https://whats.todaysplan.com.au"
    response.info.apis.trainingpeaks_url = "https://api.trainingpeaks.com"
    response.info.time = int(get_utc_time())
    udp_node = response.info.nodes.nodes.add()
    if request.remote_addr == '127.0.0.1':  # to avoid needing hairpinning
        udp_node.ip = "127.0.0.1"
    else:
        udp_node.ip = server_ip  # TCP telemetry server
    udp_node.port = 3023
    response.relay_session_id = player_id
    response.expiration = 70
    return response.SerializeToString(), 200


@app.route('/relay/session/refresh', methods=['POST'])
@jwt_to_session_cookie
@login_required
def relay_session_refresh():
    refresh = login_pb2.RelaySessionRefreshResponse()
    refresh.relay_session_id = current_user.player_id
    refresh.expiration = 70
    return refresh.SerializeToString(), 200


def logout_player(player_id):
    #Remove player from online when leaving game/world
    if player_id in online:
        online.pop(player_id)
        discord.change_presence(len(online))
    if player_id in global_ghosts:
        del global_ghosts[player_id].rec.states[:]
        global_ghosts[player_id].play.clear()
        global_ghosts.pop(player_id)
    if player_id in player_partial_profiles:
        player_partial_profiles.pop(player_id)

@app.route('/api/users/logout', methods=['POST'])
@jwt_to_session_cookie
@login_required
def api_users_logout():
    return '', 204


@app.route('/api/analytics/event', methods=['POST'])
def api_analytics_event():
    #print(json.dumps(request.json, indent=4))
    return '', 200


@app.route('/api/per-session-info', methods=['GET'])
def api_per_session_info():
    info = per_session_info_pb2.PerSessionInfo()
    info.relay_url = "https://us-or-rly101.zwift.com/relay"
    return info.SerializeToString(), 200

def get_events(limit, sport):
    events_list = [('2022 Bambino Fondo', 3368626651, 6),
                   ('2022 Medio Fondo', 2900074211, 6),
                   ('2022 Gran Fondo', 1327147942, 6),
                   ('Alpe du Zwift Downhill', 1480439148, 6),
                   ('Bologna TT', 2843604888, 10),
                   ('Crit City', 947394567, 12),
                   ('Crit City Reverse', 2875658892, 12),
                   ('Gravel Mountain', 3687150686, 16),
                   ('Gravel Mountain Reverse', 2956533021, 16),
                   ('Neokyo Crit', 1127056801, 13),
                   ('The Magnificent 8', 2207442179, 6),
                   ('Ventop Downhill', 2891361683, 14),
                   ('WBR Climbing Series', 2218409282, 6),
                   ('Zwift Bambino Fondo', 3621162212, 6),
                   ('Zwift Medio Fondo', 3748780161, 6),
                   ('Zwift Gran Fondo', 242381847, 6)]
    event_id = 1000
    cnt = 0
    events = events_pb2.Events()
    eventStart = int(get_time()) * 1000 + 60000
    eventStartWT = world_time() + 60000
    if sport == 'CYCLING':
        sport = profile_pb2.Sport.CYCLING
    else:
        sport = profile_pb2.Sport.RUNNING
        event_id = 1001 #to get sport back from id
    for item in events_list:
        event = events.events.add()
        event.server_realm = udp_node_msgs_pb2.ZofflineConstants.RealmID
        event.id = event_id
        event.name = item[0]
        event.route_id = item[1] #otherwise new home screen hangs trying to find route in all (even non-existent) courses
        event.course_id = item[2]
        event.sport = sport
        event.lateJoinInMinutes = 30
        event.eventStart = eventStart
        event.visible = True
        event.overrideMapPreferences = False
        event.invisibleToNonParticipants = False
        event.description = "Auto-generated event"
        event.distanceInMeters = 0
        event.laps = 0
        event.durationInSeconds = 0
        #event.rules_id = 
        #event.jerseyHash = 
        event.eventType = events_pb2.EventType.RACE
        #event.e_f27 = 27; //<=4, ENUM?
        #event.tags = 31; // semi-colon delimited tags
        event.e_wtrl = False # WTRL (World Tactical Racing Leagues)
        cats = ('A', 'B', 'C', 'D', 'E', 'F') 
        paceValues = ((4,15), (3,4), (2,3), (1,2), (0.1,1))
        for cat in range(1,5):
            event_cat = event.category.add()
            event_cat.id = event_id + cat
            #event_cat.registrationStart = eventStart - 30 * 60000
            #event_cat.registrationStartWT = eventStartWT - 30 * 60000
            event_cat.registrationEnd = eventStart
            event_cat.registrationEndWT = eventStartWT
            #event_cat.lineUpStart = eventStart - 5 * 60000
            #event_cat.lineUpStartWT = eventStartWT - 5 * 60000
            #event_cat.lineUpEnd = eventStart
            #event_cat.lineUpEndWT = eventStartWT
            #event_cat.eventSubgroupStart = eventStart
            #event_cat.eventSubgroupStartWT = eventStartWT
            event_cat.route_id = item[1]
            event_cat.startLocation = cat
            event_cat.label = cat
            event_cat.lateJoinInMinutes = 30
            event_cat.name = "Cat. %s" % cats[cat - 1]
            event_cat.description = "#zwiftoffline"
            event_cat.course_id = event.course_id
            event_cat.paceType = 1 #1 almost everywhere, 2 sometimes
            event_cat.fromPaceValue = paceValues[cat - 1][0]
            event_cat.toPaceValue = paceValues[cat - 1][1]
            #event_cat.scode = 7; // ex: "PT3600S"
            #event_cat.rules_id = 8; // 320 and others
            event_cat.distanceInMeters = 0
            event_cat.laps = 0
            event_cat.durationInSeconds = 0
            #event_cat.jerseyHash = 36; // 493134166, tag672
            #event_cat.tags = 45; // tag746, semi-colon delimited tags eg: "fenced;3r;created_ryan;communityevent;no_kick_mode;timestamp=1603911177622"
        event_id += 1000
        cnt += 1
        if cnt > limit:
            break
    return events

@app.route('/api/events/<int:event_id>', methods=['GET'])
def api_events_id(event_id):
    if event_id % 1 == 0:
        sport = 'CYCLING'
    else:
        sport = 'RUNNING'
    events = get_events(50, sport)
    for e in events.events:
        if e.id == event_id:
            return jsonify(convert_event_to_json(e))
    return '', 200

@app.route('/api/events/search', methods=['POST'])
def api_events_search():
    limit = int(request.args.get('limit'))
    sport = request.args.get('sport', 'CYCLING')
    events = get_events(limit, sport)
    if request.headers['Accept'] == 'application/json':
        return jsonify(convert_events_to_json(events))
    else:
        return events.SerializeToString(), 200

def create_event_wat(rel_id, wa_type, pe, dest_ids):
    player_update = udp_node_msgs_pb2.WorldAttribute()
    player_update.server_realm = udp_node_msgs_pb2.ZofflineConstants.RealmID
    player_update.wa_type = wa_type
    player_update.world_time_born = world_time()
    player_update.world_time_expire = world_time() + 60000
    player_update.wa_f12 = 1
    player_update.timestamp = int(get_utc_time()*1000000)
    player_update.rel_id = current_user.player_id

    pe.rel_id = rel_id
    pe.player_id = current_user.player_id
    #optional uint64 pje_f3/ple_f3 = 3;
    player_update.payload = pe.SerializeToString()
    player_update_s = player_update.SerializeToString()

    if not current_user.player_id in dest_ids:
        dest_ids = list(dest_ids)
        dest_ids.append(current_user.player_id)
    for receiving_player_id in dest_ids:
        enqueue_player_update(receiving_player_id, player_update_s)

@app.route('/api/events/subgroups/signup/<int:rel_id>', methods=['POST'])
@app.route('/api/events/signup/<int:rel_id>', methods=['DELETE'])
@jwt_to_session_cookie
@login_required
def api_events_subgroups_signup_id(rel_id):
    if request.method == 'POST':
        wa_type = udp_node_msgs_pb2.WA_TYPE.WAT_JOIN_E
        pe = events_pb2.PlayerJoinedEvent()
        ret = True
    else:
        wa_type = udp_node_msgs_pb2.WA_TYPE.WAT_LEFT_E
        pe = events_pb2.PlayerLeftEvent()
        ret = False
    #empty request.data
    create_event_wat(rel_id, wa_type, pe, online.keys())
    return jsonify({"signedUp":ret})

@app.route('/api/events/subgroups/register/<int:ev_sg_id>', methods=['POST'])
def api_events_subgroups_register_id(ev_sg_id):
    return '{"registered":true}', 200


@app.route('/api/events/subgroups/entrants/<int:ev_sg_id>', methods=['GET'])
def api_events_subgroups_entrants_id(ev_sg_id):
    return '[]', 200

@app.route('/api/events/subgroups/invited_ride_sweepers/<int:ev_sg_id>', methods=['GET'])
def api_events_subgroups_invited_ride_sweepers_id(ev_sg_id):
    return '[]', 200

@app.route('/api/events/subgroups/invited_ride_leaders/<int:ev_sg_id>', methods=['GET'])
def api_events_subgroups_invited_ride_leaders_id(ev_sg_id):
    return '[]', 200

@app.route('/relay/race/event_starting_line/<int:event_id>', methods=['POST'])
def relay_race_event_starting_line_id(event_id):
    return '', 204


@app.route('/api/zfiles', methods=['POST'])
@jwt_to_session_cookie
@login_required
def api_zfiles():
    zfile = zfiles_pb2.ZFileProto()
    zfile.ParseFromString(request.stream.read())
    zfiles_dir = os.path.join(STORAGE_DIR, str(current_user.player_id), zfile.folder)
    try:
        if not os.path.isdir(zfiles_dir):
            os.makedirs(zfiles_dir)
    except IOError as e:
        logger.error("failed to create zfiles dir (%s):  %s", zfiles_dir, str(e))
        return '', 400
    with open(os.path.join(zfiles_dir, quote(zfile.filename, safe=' ')), 'wb') as fd:
        fd.write(zfile.file)
    row = Zfile.query.filter_by(folder=zfile.folder, filename=zfile.filename, player_id=current_user.player_id).first()
    if not row:
        zfile.timestamp = int(get_utc_time())
        new_zfile = Zfile(folder=zfile.folder, filename=zfile.filename, timestamp=zfile.timestamp, player_id=current_user.player_id)
        db.session.add(new_zfile)
        db.session.commit()
        zfile.id = new_zfile.id
    else:
        zfile.id = row.id
        zfile.timestamp = row.timestamp
    return zfile.SerializeToString(), 200

@app.route('/api/zfiles/list', methods=['GET'])
@jwt_to_session_cookie
@login_required
def api_zfiles_list():
    folder = request.args.get('folder')
    zfiles = zfiles_pb2.ZFilesProto()
    rows = Zfile.query.filter_by(folder=folder, player_id=current_user.player_id)
    for row in rows:
        zfiles.zfiles.add(id=row.id, folder=row.folder, filename=row.filename, timestamp=row.timestamp)
    return zfiles.SerializeToString(), 200

@app.route('/api/zfiles/<int:file_id>/download', methods=['GET'])
@jwt_to_session_cookie
@login_required
def api_zfiles_download(file_id):
    row = Zfile.query.filter_by(id=file_id).first()
    zfile = os.path.join(STORAGE_DIR, str(row.player_id), row.folder, quote(row.filename, safe=' '))
    if os.path.isfile(zfile):
        return send_file(zfile, as_attachment=True, download_name=row.filename)
    else:
        return '', 404

@app.route('/api/zfiles/<int:file_id>', methods=['DELETE'])
@jwt_to_session_cookie
@login_required
def api_zfiles_delete(file_id):
    row = Zfile.query.filter_by(id=file_id).first()
    try:
        os.remove(os.path.join(STORAGE_DIR, str(row.player_id), row.folder, quote(row.filename, safe=' ')))
    except Exception as exc:
        logger.warn('api_zfiles_delete: %s' % repr(exc))
    db.session.delete(row)
    db.session.commit()
    return '', 200


# Custom static data
@app.route('/style/<path:filename>')
def custom_style(filename):
    return send_from_directory('%s/cdn/style' % SCRIPT_DIR, filename)


# Launcher files are requested over https on macOS
@app.route('/static/web/launcher/<path:filename>')
def static_web_launcher(filename):
    return send_from_directory('%s/cdn/static/web/launcher' % SCRIPT_DIR, filename)


@app.route('/api/telemetry/config', methods=['GET'])
def api_telemetry_config():
    return jsonify({"analyticsEvents": True, "batchInterval": 120, "innermostCullingRadius": 1500, "isEnabled": True,
        "key": "aXBSdlpza3p1aVlNOENrMTBQSzZEZ004Z2pwRm8zZUE6", "remoteLogLevel": 3, "sampleInterval": 60,
        "url": "https://us-or-rly101.zwift.com/v1/track", # used if no urlBatch (https://api.segment.io/v1/track)
        "urlBatch": "https://us-or-rly101.zwift.com/hvc-ingestion-service/batch"})

@app.route('/v1/track', methods=['POST'])
@app.route('/hvc-ingestion-service/batch', methods=['POST'])
def hvc_ingestion_service_batch():
    #print(json.dumps(request.json, indent=4))
    return jsonify({"success": True})


def age(dob):
    today = datetime.date.today()
    years = today.year - dob.year
    if today.month < dob.month or (today.month == dob.month and today.day < dob.day):
        years -= 1
    return years

def jsf(obj, field, deflt = None):
    if(obj.HasField(field)):
        return getattr(obj, field)
    return deflt

def jsb0(obj, field):
    return jsf(obj, field, False)

def jsb1(obj, field):
    return jsf(obj, field, True)

def jsv0(obj, field):
    return jsf(obj, field, 0)

def jses(obj, field):
    return str(jsf(obj, field))

def copyAttributes(jprofile, jprofileFull, src):
    dict = jprofileFull.get(src)
    if dict is None:
        return
    dest = {}
    for di in dict:
        for v in ['numberValue', 'floatValue', 'stringValue']:
            if v in di:
                dest[di['id']] = di[v]
    jprofile[src] = dest

def powerSourceModelToStr(val):
    if (val == 1):
        return "Power Meter"
    else:
        return "zPower"

def privacy(profile):
    privacy_bits = jsf(profile, 'privacy_bits', 0)
    return {"approvalRequired": bool(privacy_bits & 1), "displayWeight": bool(privacy_bits & 4), "minor": bool(privacy_bits & 2), "privateMessaging": bool(privacy_bits & 8), "defaultFitnessDataPrivacy": bool(privacy_bits & 16), 
"suppressFollowerNotification": bool(privacy_bits & 32), "displayAge": not bool(privacy_bits & 64), "defaultActivityPrivacy": profile_pb2.ActivityPrivacyType.Name(jsv0(profile, 'default_activity_privacy'))}

def bikeFrameToStr(val):
    item = GD.find("./BIKEFRAMES/BIKEFRAME[@signature='%s']" % val)
    if item is not None:
        return item.get('name')
    return "---"

def do_api_profiles(profile_id, is_json):
    profile_dir = '%s/%s' % (STORAGE_DIR, profile_id)
    try:
        if not os.path.isdir(profile_dir):
            os.makedirs(profile_dir)
    except IOError as e:
        logger.error("failed to create profile dir (%s):  %s", profile_dir, str(e))
        return '', 500
    profile = profile_pb2.PlayerProfile()
    profile_file = '%s/profile.bin' % profile_dir
    if not os.path.isfile(profile_file):
        profile.id = profile_id
        profile.email = current_user.username
        profile.first_name = current_user.first_name
        profile.last_name = current_user.last_name
    else: 
        with open(profile_file, 'rb') as fd:
            profile.ParseFromString(fd.read())
            profile.id = profile_id
            if not profile.email:
                profile.email = 'user@email.com'
            if profile.entitlements:
                del profile.entitlements[:]
            if not profile.mix_panel_distinct_id:
                profile.mix_panel_distinct_id = str(uuid.uuid4())
    if is_json: #todo: publicId, bodyType, totalRunCalories != total_watt_hours, totalRunTimeInMinutes != time_ridden_in_minutes etc
        if profile.dob != "":
            profile.age = age(datetime.datetime.strptime(profile.dob, "%m/%d/%Y"))
        jprofileFull = MessageToDict(profile)
        jprofile = {"id": profile.id, "firstName": jsf(profile, 'first_name'), "lastName": jsf(profile, 'last_name'), "preferredLanguage": jsf(profile, 'preferred_language'), "bodyType":jsv0(profile, 'body_type'), "male": jsb1(profile, 'is_male'), 
"imageSrc": imageSrc(profile.id), "imageSrcLarge": imageSrc(profile.id), "playerType": profile_pb2.PlayerType.Name(jsf(profile, 'player_type', 1)), "playerTypeId": jsf(profile, 'player_type', 1), "playerSubTypeId": None, 
"emailAddress": jsf(profile, 'email'), "countryCode": jsf(profile, 'country_code'), "dob": jsf(profile, 'dob'), "countryAlpha3": "rus", "useMetric": jsb1(profile, 'use_metric'), "privacy": privacy(profile), "age": jsv0(profile, 'age'), 
"ftp": jsf(profile, 'ftp'), "b": False, "weight": jsf(profile, 'weight_in_grams'), "connectedToStrava": jsb0(profile, 'connected_to_strava'), "connectedToTrainingPeaks": jsb0(profile, 'connected_to_training_peaks'), 
"connectedToTodaysPlan": jsb0(profile, 'connected_to_todays_plan'), "connectedToUnderArmour": jsb0(profile, 'connected_to_under_armour'), "connectedToFitbit": jsb0(profile, 'connected_to_fitbit'), "connectedToGarmin": jsb0(profile, 'connected_to_garmin'), "height": jsf(profile, 'height_in_millimeters'), "location": "", 
"totalExperiencePoints": jsv0(profile, 'total_xp'), "worldId": jsf(profile, 'server_realm'), "totalDistance": jsv0(profile, 'total_distance_in_meters'), "totalDistanceClimbed": jsv0(profile, 'elevation_gain_in_meters'), "totalTimeInMinutes": jsv0(profile, 'time_ridden_in_minutes'), 
"achievementLevel": jsv0(profile, 'achievement_level'), "totalWattHours": jsv0(profile, 'total_watt_hours'), "runTime1miInSeconds": jsv0(profile, 'run_time_1mi_in_seconds'), "runTime5kmInSeconds": jsv0(profile, 'run_time_5km_in_seconds'), "runTime10kmInSeconds": jsv0(profile, 'run_time_10km_in_seconds'), 
"runTimeHalfMarathonInSeconds": jsv0(profile, 'run_time_half_marathon_in_seconds'), "runTimeFullMarathonInSeconds": jsv0(profile, 'run_time_full_marathon_in_seconds'), "totalInKomJersey": jsv0(profile, 'total_in_kom_jersey'), "totalInSprintersJersey": jsv0(profile, 'total_in_sprinters_jersey'), 
"totalInOrangeJersey": jsv0(profile, 'total_in_orange_jersey'), "currentActivityId": jsf(profile, 'current_activity_id'), "enrolledZwiftAcademy": jsv0(profile, 'enrolled_program') == profile.EnrolledProgram.ZWIFT_ACADEMY, "runAchievementLevel": jsv0(profile, 'run_achievement_level'), 
"totalRunDistance": jsv0(profile, 'total_run_distance'), "totalRunTimeInMinutes": jsv0(profile, 'total_run_time_in_minutes'), "totalRunExperiencePoints": jsv0(profile, 'total_run_experience_points'), "totalRunCalories": jsv0(profile, 'total_run_calories'), "totalGold": jsv0(profile, 'total_gold_drops'), 
"profilePropertyChanges": jprofileFull.get('propertyChanges'), "cyclingOrganization": jsf(profile, 'cycling_organization'), "userAgent": "CNL/3.13.0 (Android 11) zwift/1.0.85684 curl/7.78.0-DEV", "stravaPremium": jsb0(profile, 'strava_premium'), "profileChanges": False, "launchedGameClient": "09/19/2021 13:24:19 +0000", 
"createdOn":"2021-09-19T13:24:17.783+0000", "likelyInGame": False, "address": None, "bt":"f97803d3-efac-4510-a17a-ef44e65d3071", "numberOfFolloweesInCommon": 0, "fundraiserId": None, "source": "Android", "origin": None, "licenseNumber": None, "bigCommerceId": None, "marketingConsent": None, "affiliate": None, 
"avantlinkId": None, "virtualBikeModel": bikeFrameToStr(profile.bike_frame), "connectedToWithings": jsb0(profile, 'connected_to_withings'), "connectedToRuntastic": jsb0(profile, 'connected_to_runtastic'), "connectedToZwiftPower": False, "powerSourceType": "Power Source", 
"powerSourceModel": powerSourceModelToStr(profile.power_source_model), "riding": False, "location": "", "publicId": "5a72e9b1-239f-435e-8757-af9467336b40", "mixpanelDistinctId": "21304417-af2d-4c9b-8543-8ba7c0500e84"}
        copyAttributes(jprofile, jprofileFull, 'publicAttributes')
        copyAttributes(jprofile, jprofileFull, 'privateAttributes')
        return jsonify(jprofile)
    else:
        return profile.SerializeToString(), 200

@app.route('/api/profiles/me', methods=['GET'], strict_slashes=False)
@jwt_to_session_cookie
@login_required
def api_profiles_me():
    if(request.headers['Source'] == "zwift-companion"):
        return do_api_profiles(current_user.player_id, True)
    else:
        return do_api_profiles(current_user.player_id, False)

@app.route('/api/profiles/<int:profile_id>', methods=['GET'])
@jwt_to_session_cookie
@login_required
def api_profiles_json(profile_id):
    return do_api_profiles(profile_id, True)

@app.route('/api/partners/garmin/auth', methods=['GET'])
@app.route('/api/partners/trainingpeaks/auth', methods=['GET'])
@app.route('/api/partners/strava/auth', methods=['GET'])
@app.route('/api/partners/withings/auth', methods=['GET'])
@app.route('/api/partners/todaysplan/auth', methods=['GET'])
@app.route('/api/partners/runtastic/auth', methods=['GET'])
@app.route('/api/partners/underarmour/auth', methods=['GET'])
@app.route('/api/partners/fitbit/auth', methods=['GET'])
def api_profiles_partners():
    return {"status":"notConnected","clientId":"zwift","sandbox":False}

@app.route('/api/profiles/<int:player_id>/privacy', methods=['POST'])
@jwt_to_session_cookie
@login_required
def api_profiles_id_privacy(player_id):
    privacy_file = '%s/%s/privacy.json' % (STORAGE_DIR, player_id)
    jp = request.get_json()
    with open(privacy_file, 'w', encoding='utf-8') as fprivacy:
        fprivacy.write(json.dumps(jp, ensure_ascii=False))
    #{"displayAge": false, "defaultActivityPrivacy": "PUBLIC", "approvalRequired": false, "privateMessaging": false, "defaultFitnessDataPrivacy": false}
    profile_dir = '%s/%s' % (STORAGE_DIR, player_id)
    profile = profile_pb2.PlayerProfile()
    profile_file = '%s/profile.bin' % profile_dir
    with open(profile_file, 'rb') as fd:
        profile.ParseFromString(fd.read())
    profile.privacy_bits = 0
    if (jp["approvalRequired"]):
        profile.privacy_bits += 1
    if ("displayWeight" in jp and jp["displayWeight"]):
        profile.privacy_bits += 4
    if ("minor" in jp and jp["minor"]):
        profile.privacy_bits += 2
    if (jp["privateMessaging"]):
        profile.privacy_bits += 8
    if (jp["defaultFitnessDataPrivacy"]):
        profile.privacy_bits += 16
    if ("suppressFollowerNotification" in jp and jp["suppressFollowerNotification"]):
        profile.privacy_bits += 32
    if (not jp["displayAge"]):
        profile.privacy_bits += 64
    defaultActivityPrivacy = jp["defaultActivityPrivacy"]
    profile.default_activity_privacy = 0 #PUBLIC
    if(defaultActivityPrivacy == "PRIVATE"):
        profile.default_activity_privacy = 1
    if(defaultActivityPrivacy == "FRIENDS"):
        profile.default_activity_privacy = 2
    with open(profile_file, 'wb') as fd:
        fd.write(profile.SerializeToString())
    return '', 200

@app.route('/api/profiles/<int:m_player_id>/followers', methods=['GET']) #?start=0&limit=200&include-follow-requests=false
@app.route('/api/profiles/<int:m_player_id>/followees', methods=['GET'])
@app.route('/api/profiles/<int:m_player_id>/followees-in-common/<int:t_player_id>', methods=['GET'])
@jwt_to_session_cookie
@login_required
def api_profiles_followers(m_player_id, t_player_id=0):
    rows = db.session.execute("SELECT player_id, first_name, last_name FROM user")
    json_data_list = []
    for row in rows:
        player_id = row[0]
        profile = get_partial_profile(player_id)
        #all users are following favourites of this user (temp decision for small crouds)
        json_data_list.append({"id":0,"followerId":player_id,"followeeId":m_player_id,"status":"IS_FOLLOWING","isFolloweeFavoriteOfFollower":True, \
            "followerProfile":{"id":player_id,"firstName":row[1],"lastName":row[2],"imageSrc":imageSrc(player_id),"imageSrcLarge":imageSrc(player_id),"countryCode":profile.country_code}, \
            "followeeProfile":None})
    return jsonify(json_data_list)

@app.route('/api/search/profiles/restricted', methods=['POST'])
@app.route('/api/search/profiles', methods=['POST'])
@jwt_to_session_cookie
@login_required
def api_search_profiles():
    query = request.json['query']
    start = request.args.get('start')
    limit = request.args.get('limit')
    stmt = sqlalchemy.text("SELECT player_id, first_name, last_name FROM user WHERE first_name LIKE :n OR last_name LIKE :n LIMIT :l OFFSET :o")
    rows = db.session.execute(stmt, {"n": "%"+query+"%", "l": limit, "o": start})
    json_data_list = []
    for row in rows:
        player_id = row[0]
        profile = get_partial_profile(player_id)
        json_data_list.append({"id": player_id, "firstName": row[1], "lastName": row[2], "imageSrc": imageSrc(player_id), "imageSrcLarge": imageSrc(player_id), "countryCode": profile.country_code})
    return jsonify(json_data_list)

@app.route('/api/profiles/<int:player_id>/statistics', methods=['GET'])
def api_profiles_id_statistics(player_id):
    from_dt = request.args.get('startDateTime')
    stmt = sqlalchemy.text("SELECT SUM(CAST((julianday(date)-julianday(start_date))*24*60 AS integer)), SUM(distanceInMeters), SUM(calories), SUM(total_elevation) FROM activity WHERE player_id = :p AND strftime('%s', start_date) >= strftime('%s', :d)")
    row = db.session.execute(stmt, {"p": player_id, "d": from_dt}).first()
    json_data = {"timeRiddenInMinutes": row[0], "distanceRiddenInMeters": row[1], "caloriesBurned": row[2], "heightClimbedInMeters": row[3]}
    return jsonify(json_data)

@app.route('/relay/profiles/me/phone', methods=['PUT'])
@jwt_to_session_cookie
@login_required
def api_profiles_me_phone():
    global zc_connect_queue
    if not request.stream:
        return '', 400
    phoneAddress = request.json['phoneAddress']
    if 'port' in request.json:
        phonePort = int(request.json['port'])
        phoneSecretKey = 'None'
    if 'securePort' in request.json:
        phonePort = int(request.json['securePort'])
        phoneSecretKey = base64.b64decode(request.json['secret'])
    zc_connect_queue[current_user.player_id] = (phoneAddress, phonePort, phoneSecretKey)
    #todo UDP scenario
    #logger.info("ZCompanion %d reg: %s:%d (key: %s)" % (current_user.player_id, phoneAddress, phonePort, phoneSecretKey.hex()))
    return '', 204

@app.route('/api/profiles/me/<int:player_id>', methods=['PUT'])
@jwt_to_session_cookie
@login_required
def api_profiles_me_id(player_id):
    if not request.stream:
        return '', 400
    if current_user.player_id != player_id:
        return '', 401
    profile_dir = '%s/%s' % (STORAGE_DIR, player_id)
    profile = profile_pb2.PlayerProfile()
    profile_file = '%s/profile.bin' % profile_dir
    with open(profile_file, 'rb') as fd:
        profile.ParseFromString(fd.read())
    #update profile from json
    profile.country_code = request.json['countryCode']
    profile.dob = request.json['dob']
    profile.email = request.json['emailAddress']
    profile.first_name = request.json['firstName']
    profile.last_name = request.json['lastName']
    profile.height_in_millimeters = request.json['height']
    profile.is_male = request.json['male']
    profile.use_metric = request.json['useMetric']
    profile.weight_in_grams = request.json['weight']
    image = imageSrc(player_id)
    if image is not None:
        profile.large_avatar_url = image
    with open(profile_file, 'wb') as fd:
        fd.write(profile.SerializeToString())
    if MULTIPLAYER:
        current_user.first_name = profile.first_name
        current_user.last_name = profile.last_name
        db.session.commit()
    return api_profiles_me()

@app.route('/api/profiles/<int:player_id>', methods=['PUT'])
@app.route('/api/profiles/<int:player_id>/in-game-fields', methods=['PUT'])
@jwt_to_session_cookie
@login_required
def api_profiles_id(player_id):
    if not request.stream:
        return '', 400
    if current_user.player_id != player_id:
        return '', 401
    stream = request.stream.read()
    with open('%s/%s/profile.bin' % (STORAGE_DIR, player_id), 'wb') as f:
        f.write(stream)
    if MULTIPLAYER:
        profile = profile_pb2.PlayerProfile()
        profile.ParseFromString(stream)
        current_user.first_name = profile.first_name
        current_user.last_name = profile.last_name
        db.session.commit()
    return '', 204

@app.route('/api/profiles/<int:player_id>/photo', methods=['POST'])
@jwt_to_session_cookie
@login_required
def api_profiles_id_photo_post(player_id):
    if not request.stream:
        return '', 400
    if current_user.player_id != player_id:
        return '', 401
    stream = request.stream.read().split(b'\r\n\r\n', maxsplit=1)[1]
    with open('%s/%s/avatarLarge.jpg' % (STORAGE_DIR, player_id), 'wb') as f:
        f.write(stream)
    return '', 200

@app.route('/api/profiles/<int:player_id>/activities', methods=['GET', 'POST'], strict_slashes=False)
@jwt_to_session_cookie
@login_required
def api_profiles_activities(player_id):
    if request.method == 'POST':
        if not request.stream:
            return '', 400
        if current_user.player_id != player_id:
            return '', 401
        activity = activity_pb2.Activity()
        activity.ParseFromString(request.stream.read())
        activity.id = insert_protobuf_into_db(Activity, activity, ['fit'])
        return '{"id": %ld}' % activity.id, 200

    # request.method == 'GET'
    activities = activity_pb2.ActivityList()
    rows = db.session.execute(sqlalchemy.text("SELECT * FROM activity WHERE player_id = :p"), {"p": player_id})
    should_remove = list()
    for row in rows:
        activity = activities.activities.add()
        row_to_protobuf(row, activity, exclude_fields=['fit'])
        #Remove activities with less than 100m distance
        if activity.distanceInMeters < 100:
            should_remove.append(activity)
    for a in should_remove:
        db.session.execute(sqlalchemy.text("DELETE FROM activity WHERE id = :i"), {"i": a.id})
        db.session.commit()
        activities.activities.remove(a)
    return activities.SerializeToString(), 200

def time_since(state):
    seconds = (world_time() - state.worldTime) // 1000
    interval = seconds // 31536000
    if interval > 0: interval_type = 'year'
    else:
        interval = seconds // 2592000
        if interval > 0: interval_type = 'month'
        else:
            interval = seconds // 604800
            if interval > 0: interval_type = 'week'
            else:
                interval = seconds // 86400
                if interval > 0: interval_type = 'day'
                else:
                    interval = seconds // 3600
                    if interval > 0: interval_type = 'hour'
                    else:
                        interval = seconds // 60
                        if interval > 0: interval_type = 'minute'
                        else: return 'Just now'
    if interval > 1: interval_type += 's'
    return '%s %s ago' % (interval, interval_type)

def random_profile(p):
    p.ride_helmet_type = int(random.choice(GD.findall("./HEADGEARS/HEADGEAR")).get('signature'))
    p.glasses_type = int(random.choice(GD.findall("./GLASSES/GLASS")).get('signature'))
    p.ride_shoes_type = int(random.choice(GD.findall("./BIKESHOES/BIKESHOE")).get('signature'))
    p.ride_socks_type = int(random.choice(GD.findall("./SOCKS/SOCK")).get('signature'))
    p.ride_jersey = int(random.choice(GD.findall("./JERSEYS/JERSEY")).get('signature'))
    p.bike_wheel_front = int(random.choice(GD.findall("./BIKEFRONTWHEELS/BIKEFRONTWHEEL")).get('signature'))
    p.bike_wheel_rear = int(random.choice(GD.findall("./BIKEREARWHEELS/BIKEREARWHEEL")).get('signature'))
    p.bike_frame = int(random.choice(GD.findall("./BIKEFRAMES/BIKEFRAME")).get('signature'))
    p.run_shirt_type = int(random.choice(GD.findall("./RUNSHIRTS/RUNSHIRT")).get('signature'))
    p.run_shorts_type = int(random.choice(GD.findall("./RUNSHORTS/RUNSHORT")).get('signature'))
    p.run_shoes_type = int(random.choice(GD.findall("./RUNSHOES/RUNSHOE")).get('signature'))
    return p

@app.route('/api/profiles', methods=['GET'])
def api_profiles():
    args = request.args.getlist('id')
    profiles = profile_pb2.PlayerProfiles()
    for i in args:
        p_id = int(i)
        profile = profile_pb2.PlayerProfile()
        if p_id > 10000000:
            ghostId = math.floor(p_id / 10000000)
            player_id = p_id - ghostId * 10000000
            profile_file = '%s/%s/profile.bin' % (STORAGE_DIR, player_id)
            if os.path.isfile(profile_file):
                with open(profile_file, 'rb') as fd:
                    profile.ParseFromString(fd.read())
                    p = profiles.profiles.add()
                    p.CopyFrom(random_profile(profile))
                    p.id = p_id
                    p.first_name = ''
                    p.last_name = time_since(global_ghosts[player_id].play[ghostId-1].states[0])
                    p.country_code = 0
        else:
            if p_id in global_pace_partners.keys():
                profile = global_pace_partners[p_id].profile
            elif p_id in global_bots.keys():
                profile = global_bots[p_id].profile
            else:
                profile_file = '%s/%s/profile.bin' % (STORAGE_DIR, p_id)
                if os.path.isfile(profile_file):
                    try:
                        with open(profile_file, 'rb') as fd:
                            profile.ParseFromString(fd.read())
                    except Exception as exc:
                        logger.warn('api_profiles: %s' % repr(exc))
            profiles.profiles.append(profile)
    return profiles.SerializeToString(), 200

@app.route('/api/player-playbacks/player/playback', methods=['POST'])
@jwt_to_session_cookie
@login_required
def player_playbacks_player_playback():
    pb_dir = '%s/playbacks' % STORAGE_DIR
    try:
        if not os.path.isdir(pb_dir):
            os.makedirs(pb_dir)
    except IOError as e:
        logger.error("failed to create playbacks dir (%s):  %s", pb_dir, str(e))
        return '', 400
    stream = request.stream.read()
    pb = playback_pb2.Playback()
    pb.ParseFromString(stream)
    if pb.time == 0:
        return '', 200
    new_uuid = str(uuid.uuid4())
    new_pb = Playback(player_id=current_user.player_id, uuid=new_uuid, segment_id=pb.segment_id, time=pb.time, world_time=pb.world_time)
    db.session.add(new_pb)
    db.session.commit()
    with open('%s/%s.playback' % (pb_dir, new_uuid), 'wb') as f:
        f.write(stream)
    return '', 200

@app.route('/api/player-playbacks/player/me/playbacks/<segment_id>/<option>', methods=['GET'])
@jwt_to_session_cookie
@login_required
def player_playbacks_player_me_playbacks(segment_id, option):
    segment_id = int(segment_id)
    after = request.args.get('after')
    before = request.args.get('before')
    query = "SELECT * FROM playback WHERE player_id = :p AND segment_id = :s"
    args = {"p": current_user.player_id, "s": segment_id}
    if after:
        query += " AND world_time > :a"
        args.update({"a": after})
    if before:
        query += " AND world_time < :b"
        args.update({"b": before})
    if option == 'pr':
        query += " ORDER BY time"
    elif option == 'latest':
        query += " ORDER BY world_time DESC"
    row = db.session.execute(sqlalchemy.text(query), args).first()
    if not row:
        return '', 200
    pbr = playback_pb2.PlaybackResponse()
    pbr.uuid = row.uuid
    pbr.segment_id = row.segment_id
    pbr.time = row.time
    pbr.world_time = row.world_time
    pbr.url = 'https://cdn.zwift.com/player-playback/playbacks/%s.playback' % row.uuid
    return pbr.SerializeToString(), 200

@app.route('/player-playback/playbacks/<path:filename>')
def player_playback_playbacks(filename):
    return send_from_directory('%s/playbacks' % STORAGE_DIR, filename)

@app.route('/route-results', methods=['POST'])
@jwt_to_session_cookie
@login_required
def route_results():
    return '', 200

def strava_upload(player_id, activity):
    try:
        from stravalib.client import Client
    except ImportError as exc:
        logger.warn('stravalib: %s' % repr(exc))
        logger.warn("stravalib is not installed. Skipping Strava upload attempt.")
        return
    profile_dir = '%s/%s' % (STORAGE_DIR, player_id)
    strava_token = '%s/strava_token.txt' % profile_dir
    if not os.path.exists(strava_token):
        logger.info("strava_token.txt missing, skip Strava activity update")
        return
    strava = Client()
    try:
        with open(strava_token, 'r') as f:
            client_id = f.readline().rstrip('\r\n')
            client_secret = f.readline().rstrip('\r\n')
            strava.access_token = f.readline().rstrip('\r\n')
            refresh_token = f.readline().rstrip('\r\n')
            expires_at = f.readline().rstrip('\r\n')
    except Exception as exc:
        logger.warn("Failed to read %s. Skipping Strava upload attempt. %s" % (strava_token, repr(exc)))
        return
    try:
        if get_utc_time() > int(expires_at):
            refresh_response = strava.refresh_access_token(client_id=client_id, client_secret=client_secret,
                                                           refresh_token=refresh_token)
            with open(strava_token, 'w') as f:
                f.write(client_id + '\n')
                f.write(client_secret + '\n')
                f.write(refresh_response['access_token'] + '\n')
                f.write(refresh_response['refresh_token'] + '\n')
                f.write(str(refresh_response['expires_at']) + '\n')
    except Exception as exc:
        logger.warn("Failed to refresh token. Skipping Strava upload attempt: %s." % repr(exc))
        return
    try:
        # See if there's internet to upload to Strava
        strava.upload_activity(BytesIO(activity.fit), data_type='fit', name=activity.name)
        # XXX: assume the upload succeeds on strava's end. not checking on it.
    except Exception as exc:
        logger.warn("Strava upload failed. No internet? %s" % repr(exc))


def garmin_upload(player_id, activity):
    try:
        from garmin_uploader.workflow import Workflow
    except ImportError as exc:
        logger.warn("garmin_uploader is not installed. Skipping Garmin upload attempt. %s" % repr(exc))
        return
    profile_dir = '%s/%s' % (STORAGE_DIR, player_id)
    garmin_credentials = '%s/garmin_credentials' % profile_dir
    if os.path.exists(garmin_credentials + '.bin'):
        garmin_credentials += '.bin'
    elif os.path.exists(garmin_credentials + '.txt'):
        garmin_credentials += '.txt'
    else:
        logger.info("garmin_credentials missing, skip Garmin activity update")
        return
    try:
        if garmin_credentials.endswith('.bin'):
            with open(garmin_credentials, 'rb') as f:
                iv = f.read(16)
                ciphered_text = f.read()
                cipher_suite = AES.new(credentials_key, AES.MODE_CFB, iv=iv)
                unciphered_text = cipher_suite.decrypt(ciphered_text).decode('UTF-8')
                split_credentials = unciphered_text.splitlines()
                username = split_credentials[0]
                password = split_credentials[1]
        else:
            with open(garmin_credentials) as f:
                username = f.readline().rstrip('\r\n')
                password = f.readline().rstrip('\r\n')
    except Exception as exc:
        logger.warn("Failed to read %s. Skipping Garmin upload attempt. %s" % (garmin_credentials, repr(exc)))
        return
    try:
        with open('%s/last_activity.fit' % profile_dir, 'wb') as f:
            f.write(activity.fit)
    except Exception as exc:
        logger.warn("Failed to save fit file. Skipping Garmin upload attempt. %s" % repr(exc))
        return
    try:
        w = Workflow(['%s/last_activity.fit' % profile_dir], activity_name=activity.name, username=username, password=password)
        w.run()
    except Exception as exc:
        logger.warn("Garmin upload failed. No internet? %s" % repr(exc))

def runalyze_upload(player_id, activity):
    profile_dir = '%s/%s' % (STORAGE_DIR, player_id)
    runalyze_token = '%s/runalyze_token.txt' % profile_dir
    if not os.path.exists(runalyze_token):
        logger.info("runalyze_token.txt missing, skip Runalyze activity update")
        return
    try:
        with open(runalyze_token, 'r') as f:
            runtoken = f.readline().rstrip('\r\n')
    except Exception as exc:
        logger.warn("Failed to read %s. Skipping Runalyze upload attempt." % (runalyze_token, repr(exc)))
        return
    try:
        with open('%s/last_activity.fit' % profile_dir, 'wb') as f:
            f.write(activity.fit)
    except Exception as exc:
        logger.warn("Failed to save fit file. Skipping Runalyze upload attempt. %s" % repr(exc))
        return
    try:
        r = requests.post("https://runalyze.com/api/v1/activities/uploads",
                          files={'file': open('%s/last_activity.fit' % profile_dir, "rb")},
                          headers={"token": runtoken})
        logger.info(r.text)
    except Exception as exc:
        logger.warn("Runalyze upload failed. No internet? %s" % repr(exc))


def zwift_upload(player_id, activity):
    profile_dir = '%s/%s' % (STORAGE_DIR, player_id)
    zwift_credentials = '%s/zwift_credentials.bin' % profile_dir
    if not os.path.exists(zwift_credentials):
        logger.info("zwift_credentials.bin missing, skip Zwift activity update")
        return
    if not os.path.exists(SERVER_IP_FILE):
        logger.info("server_ip.txt missing, skip Zwift activity update")
        return
    try:
        with open(zwift_credentials, 'rb') as f:
            iv = f.read(16)
            ciphered_text = f.read()
            cipher_suite = AES.new(credentials_key, AES.MODE_CFB, iv=iv)
            unciphered_text = cipher_suite.decrypt(ciphered_text).decode('UTF-8')
            split_credentials = unciphered_text.splitlines()
            username = split_credentials[0]
            password = split_credentials[1]
    except Exception as exc:
        logger.warn("Failed to read %s. Skipping Zwift upload attempt. %s" % (zwift_credentials, repr(exc)))
        return
    
    try:
        session = requests.session()
        try:
            access_token, refresh_token = online_sync.login(session, username, password)
            activity.player_id = online_sync.get_player_id(session, access_token)
            res = online_sync.upload_activity(session, access_token, activity)
            if res == 200:
                logger.info("Zwift activity upload succesfull")
            else:
                logger.warn("Zwift activity upload failed:%s:" %res)
            online_sync.logout(session, refresh_token)
        except Exception as exc:
            logger.warn("Error uploading activity to Zwift Server. %s" % repr(exc))
    except Exception as exc:
        logger.warn("Zwift upload failed. No internet? %s" % repr(exc))


@app.route('/api/profiles/<int:player_id>/activities/<int:activity_id>', methods=['PUT', 'DELETE'])
@jwt_to_session_cookie
@login_required
def api_profiles_activities_id(player_id, activity_id):
    if not request.stream:
        return '', 400
    if current_user.player_id != player_id:
        return '', 401
    if request.method == 'DELETE':
        db.session.execute(sqlalchemy.text("DELETE FROM activity WHERE id = :i"), {"i": activity_id})
        db.session.commit()
        logout_player(player_id)
        return 'true', 200
    activity = activity_pb2.Activity()
    activity.ParseFromString(request.stream.read())
    update_protobuf_in_db(Activity, activity, activity_id, ['fit'])
    fit_filename = '%s - %s' % (activity_id, activity.fit_filename)
    save_fit(player_id, fit_filename, activity.fit)

    response = '{"id":%s}' % activity_id
    if request.args.get('upload-to-strava') != 'true':
        return response, 200
    if activity.distanceInMeters < 300:
        return response, 200
    if current_user.enable_ghosts:
        try:
            save_ghost(quote(activity.name, safe=' '), player_id)
        except Exception as exc:
            logger.warn('save_ghost: %s' % repr(exc))
    # For using with upload_activity
    with open('%s/%s/last_activity.bin' % (STORAGE_DIR, player_id), 'wb') as f:
        f.write(activity.SerializeToString())
    # Unconditionally *try* and upload to strava and garmin since profile may
    # not be properly linked to strava/garmin (i.e. no 'upload-to-strava' call
    # will occur with these profiles).
    strava_upload(player_id, activity)
    garmin_upload(player_id, activity)
    runalyze_upload(player_id, activity)
    zwift_upload(player_id, activity)
    logout_player(player_id)
    return response, 200

@app.route('/api/profiles/<int:receiving_player_id>/activities/0/rideon', methods=['POST']) #activity_id Seem to always be 0, even when giving ride on to ppl with 30km+
@jwt_to_session_cookie
@login_required
def api_profiles_activities_rideon(receiving_player_id):
    sending_player_id = request.json['profileId']
    profile = get_partial_profile(sending_player_id)
    player_update = udp_node_msgs_pb2.WorldAttribute()
    player_update.server_realm = udp_node_msgs_pb2.ZofflineConstants.RealmID
    player_update.wa_type = udp_node_msgs_pb2.WA_TYPE.WAT_RIDE_ON
    player_update.world_time_born = world_time()
    player_update.world_time_expire = player_update.world_time_born + 9890
    player_update.timestamp = int(get_utc_time() * 1000000)

    ride_on = udp_node_msgs_pb2.RideOn()
    ride_on.player_id = int(sending_player_id)
    ride_on.to_player_id = int(receiving_player_id)
    ride_on.firstName = profile.first_name
    ride_on.lastName = profile.last_name
    ride_on.countryCode = profile.country_code

    player_update.payload = ride_on.SerializeToString()

    enqueue_player_update(receiving_player_id, player_update.SerializeToString())

    receiver = get_partial_profile(receiving_player_id)
    message = 'Ride on ' + receiver.first_name + ' ' + receiver.last_name + '!'
    discord.send_message(message, sending_player_id)
    return '{}', 200

def stime_to_timestamp(stime):
    utc_offset = datetime.datetime.fromtimestamp(0) - datetime.datetime.utcfromtimestamp(0)
    return int((datetime.datetime.strptime(stime, '%Y-%m-%dT%H:%M:%SZ') + utc_offset).timestamp())

def create_zca_notification(player_id, private_event, organizer):
    orm_not = Notification(event_id=private_event['id'], player_id=player_id, json='')
    db.session.add(orm_not)
    db.session.commit()
    argString0 = json.dumps({"eventId":private_event['id'],"eventStartDate": \
        stime_to_timestamp(private_event['eventStart']), \
        "otherInviteeCount":len(private_event['invitedProfileIds'])})
    n = { "activity": None, "argLong0": 0, "argLong1": 0, "argString0": argString0,
        "createdOn": str_timestamp(unix_time_millis(datetime.datetime.now())),
        "fromProfile": {
            "firstName": organizer["firstName"],
            "id": organizer["id"],
            "imageSrc": organizer["imageSrc"],
            "imageSrcLarge": organizer["imageSrc"],
            "lastName": organizer["lastName"],
            "publicId": "283b140f-91d2-4882-bd8e-e4194ddf7128", #todo, hope not used
            "socialFacts": {
                "favoriteOfLoggedInPlayer": True, #todo
                "followeeStatusOfLoggedInPlayer": "IS_FOLLOWING", #todo
                "followerStatusOfLoggedInPlayer": "IS_FOLLOWING" #todo
            }
        },
        "id": orm_not.id, "lastModified": None, "read": False, "readDate": None,
        "type": "PRIVATE_EVENT_INVITE" 
    }
    orm_not.json = json.dumps(n)
    db.session.commit()

@app.route('/api/notifications', methods=['GET'])
@jwt_to_session_cookie
@login_required
def api_notifications():
    ret_notifications = []
    for row in Notification.query.filter_by(player_id=current_user.player_id):
        ret_notifications.append(row.json)
    return jsonify(ret_notifications)

@app.route('/api/notifications/<int:notif_id>', methods=['PUT'])
@jwt_to_session_cookie
@login_required
def api_notifications_put(notif_id):
    for orm_not in Notification.query.filter_by(id=notif_id):
        n = json.loads(orm_not.json)
        n["read"] = request.json['read']
        n["readDate"] = request.json['readDate']
        n["lastModified"] = n["readDate"]
        orm_not.json = json.dumps(n)
        db.session.commit()
    return '', 204

glb_private_events = {} #cache of actual PrivateEvent(db.Model)
def ActualPrivateEvents():
    if len(glb_private_events) == 0:
        for row in db.session.query(PrivateEvent).order_by(PrivateEvent.id.desc()).limit(100):
            if len(row.json):
                glb_private_events[row.id] = json.loads(row.json)
    return glb_private_events

@app.route('/api/private_event/<int:meetup_id>', methods=['DELETE'])
@jwt_to_session_cookie
@login_required
def api_private_event_remove(meetup_id):
    ActualPrivateEvents().pop(meetup_id)
    PrivateEvent.query.filter_by(id=meetup_id).delete()
    Notification.query.filter_by(event_id=meetup_id).delete()
    db.session.commit()
    return '', 200

def edit_private_event(player_id, meetup_id, decision):
    ape = ActualPrivateEvents()
    if meetup_id in ape.keys():
        e = ape[meetup_id]
        for i in e['eventInvites']:
            if i['invitedProfile']['id'] == player_id:
                i['status'] = decision
        orm_event = PrivateEvent.query.get(meetup_id)
        orm_event.json = json.dumps(e)
        db.session.commit()
    return '', 204

@app.route('/api/private_event/<int:meetup_id>/accept', methods=['PUT'])
@jwt_to_session_cookie
@login_required
def api_private_event_accept(meetup_id):
    return edit_private_event(current_user.player_id, meetup_id, 'ACCEPTED')

@app.route('/api/private_event/<int:meetup_id>/reject', methods=['PUT'])
@jwt_to_session_cookie
@login_required
def api_private_event_reject(meetup_id):
    return edit_private_event(current_user.player_id, meetup_id, 'REJECTED')

@app.route('/api/private_event/<int:meetup_id>', methods=['PUT'])
@jwt_to_session_cookie
@login_required
def api_private_event_edit(meetup_id):
    str_pe = request.stream.read()
    json_pe = json.loads(str_pe)
    org_json_pe = ActualPrivateEvents()[meetup_id]
    for f in ('culling', 'distanceInMeters', 'durationInSeconds', 'eventStart', 'invitedProfileIds', 'laps', 'routeId', 'rubberbanding', 'showResults', 'sport', 'workoutHash'):
        org_json_pe[f] = json_pe[f]
    org_json_pe['updateDate'] = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
    newEventInvites = []
    newEventInviteeIds = []
    for i in org_json_pe['eventInvites']:
        profile_id = i['invitedProfile']['id']
        if profile_id == org_json_pe['organizerProfileId'] or profile_id in json_pe['invitedProfileIds']:
            newEventInvites.append(i)
            newEventInviteeIds.append(profile_id)
    player_update = create_wa_event_invites(org_json_pe)
    for peer_id in json_pe['invitedProfileIds']:
        if not peer_id in newEventInviteeIds:
            create_zca_notification(peer_id, org_json_pe, newEventInvites[0]["invitedProfile"])
            player_update.rel_id = peer_id
            enqueue_wa_event_invites(peer_id, player_update)
            p_partial_profile = get_partial_profile(peer_id)
            newEventInvites.append({"invitedProfile": p_partial_profile.to_json(), "status": "PENDING"})
    org_json_pe['eventInvites'] = newEventInvites
    PrivateEvent.query.get(meetup_id).json = json.dumps(org_json_pe)
    db.session.commit()
    for orm_not in Notification.query.filter_by(event_id=meetup_id):
        n = json.loads(orm_not.json)
        n['read'] = False
        n['readDate'] = None
        n['lastModified'] = org_json_pe['updateDate']
        orm_not.json = json.dumps(n)
    db.session.commit()
    return jsonify({"id":meetup_id})

def enqueue_wa_event_invites(player_id, wa):
    for wat in (udp_node_msgs_pb2.WA_TYPE.WAT_EVENT, udp_node_msgs_pb2.WA_TYPE.WAT_INV_W):
        wa.wa_type = wat
        enqueue_player_update(player_id, wa.SerializeToString())

def create_wa_event_invites(json_pe):
    pe = events_pb2.Event()
    player_update = udp_node_msgs_pb2.WorldAttribute()
    player_update.server_realm = udp_node_msgs_pb2.ZofflineConstants.RealmID
    player_update.world_time_born = world_time()
    player_update.world_time_expire = world_time() + 60000
    player_update.wa_f12 = 1
    player_update.timestamp = int(get_utc_time()*1000000)

    pe.id = json_pe['id']
    pe.server_realm = udp_node_msgs_pb2.ZofflineConstants.RealmID
    pe.name = json_pe['name']
    if 'description' in json_pe:
        pe.description = json_pe['description']
    pe.eventStart = stime_to_timestamp(json_pe['eventStart'])*1000
    pe.distanceInMeters = json_pe['distanceInMeters']
    pe.laps = json_pe['laps']
    if 'imageUrl' in json_pe:
        pe.imageUrl = json_pe['imageUrl']
    pe.durationInSeconds = json_pe['durationInSeconds']
    pe.route_id = json_pe['routeId']
    #{"rubberbanding":true,"showResults":false,"workoutHash":0} todo_pe
    pe.visible = True
    pe.jerseyHash = 0
    pe.sport = sport_from_str(json_pe['sport'])
    #pe.uint64 e_f23 = 23; =0
    pe.eventType = events_pb2.EventType.EFONDO
    if 'culling' in json_pe:
        if json_pe['culling']:
            pe.eventType = events_pb2.EventType.RACE
    #pe.uint64 e_f25 = 25; =0
    pe.e_f27 = 2 #<=4, ENUM? saw = 2
    #pe.bool overrideMapPreferences = 28; =0
    #pe.bool invisibleToNonParticipants = 29; =0 todo_pe
    pe.lateJoinInMinutes = 30 #todo_pe
    #pe.course_id = 1 #todo_pe =f(json_pe['routeId']) ???
    player_update.payload = pe.SerializeToString()
    return player_update

@app.route('/api/private_event', methods=['POST'])
@jwt_to_session_cookie
@login_required
def api_private_event_new(): #{"culling":true,"description":"mesg","distanceInMeters":13800.0,"durationInSeconds":0,"eventStart":"2022-03-17T16:27:00Z","invitedProfileIds":[4357549,4486967],"laps":0,"routeId":2474227587,"rubberbanding":true,"showResults":false,"sport":"CYCLING","workoutHash":0}
    str_pe = request.stream.read()
    json_pe = json.loads(str_pe)

    db_pe = PrivateEvent(json=str_pe)
    db.session.add(db_pe)
    db.session.commit()

    json_pe['id'] = db_pe.id
    ev_sg_id = db_pe.id
    json_pe['eventSubgroupId'] = ev_sg_id
    json_pe['name'] = "Route #%s" % json_pe['routeId'] #todo: more readable
    json_pe['acceptedTotalCount'] = len(json_pe['invitedProfileIds']) #todo: real count
    json_pe['acceptedFolloweeCount'] = len(json_pe['invitedProfileIds']) + 1 #todo: real count
    json_pe['invitedTotalCount'] = len(json_pe['invitedProfileIds']) + 1
    partial_profile = get_partial_profile(current_user.player_id)
    json_pe['organizerProfileId'] = current_user.player_id
    json_pe['organizerId'] = current_user.player_id
    json_pe['startLocation'] = 1 #todo_pe
    json_pe['allowsLateJoin'] = True #todo_pe
    json_pe['organizerFirstName'] = partial_profile.first_name
    json_pe['organizerLastName'] = partial_profile.last_name
    json_pe['updateDate'] = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
    json_pe['organizerImageUrl'] = imageSrc(current_user.player_id)
    eventInvites = [{"invitedProfile": partial_profile.to_json(), "status": "ACCEPTED"}]
    create_event_wat(ev_sg_id, udp_node_msgs_pb2.WA_TYPE.WAT_JOIN_E, events_pb2.PlayerJoinedEvent(), online.keys())

    player_update = create_wa_event_invites(json_pe)
    enqueue_wa_event_invites(current_user.player_id, player_update)

    for peer_id in json_pe['invitedProfileIds']:
        create_zca_notification(peer_id, json_pe, eventInvites[0]["invitedProfile"])
        player_update.rel_id = peer_id
        enqueue_wa_event_invites(peer_id, player_update)
        p_partial_profile = get_partial_profile(peer_id)
        eventInvites.append({"invitedProfile": p_partial_profile.to_json(), "status": "PENDING"})
    json_pe['eventInvites'] = eventInvites

    ActualPrivateEvents()[db_pe.id] = json_pe
    db_pe.json = json.dumps(json_pe)
    db.session.commit() #update db_pe

    return jsonify({"id":db_pe.id}), 201

def clone_and_append_social(player_id, private_event):
    ret = deepcopy(private_event)
    status = 'PENDING'
    for i in ret['eventInvites']:
        p = i['invitedProfile']
        #todo: strict social
        if p['id'] == player_id:
            p['socialFacts'] = {"followerStatusOfLoggedInPlayer":"SELF","isFavoriteOfLoggedInPlayer":False}
            status = i['status']
        else:
            p['socialFacts'] = {"followerStatusOfLoggedInPlayer":"IS_FOLLOWING","isFavoriteOfLoggedInPlayer":True}
    ret['inviteStatus'] = status
    return ret

def jsonPrivateEventFeedToProtobuf(jfeed):
    ret = events_pb2.PrivateEventFeedListProto()
    for jpef in jfeed:
        pef = ret.pef.add()
        pef.event_id = jpef['id']
        pef.sport = sport_from_str(jpef['sport'])
        pef.eventSubgroupStart = stime_to_timestamp(jpef['eventStart'])*1000
        pef.route_id = jpef['routeId']
        pef.durationInSeconds = jpef['durationInSeconds']
        pef.distanceInMeters = jpef['distanceInMeters']
        pef.answeredCount = 1 #todo
        pef.invitedTotalCount = jpef['invitedTotalCount']
        pef.acceptedFolloweeCount = jpef['acceptedFolloweeCount']
        pef.acceptedTotalCount = jpef['acceptedTotalCount']
        if jpef['organizerImageUrl'] is not None:
            pef.organizerImageUrl = jpef['organizerImageUrl']
        pef.organizerProfileId = jpef['organizerProfileId']
        pef.organizerFirstName = jpef['organizerFirstName']
        pef.organizerLastName = jpef['organizerLastName']
        pef.updateDate = stime_to_timestamp(jpef['updateDate'])*1000
        pef.subgroupId = jpef['eventSubgroupId']
        pef.laps = jpef['laps']
        pef.rubberbanding = jpef['rubberbanding']
    return ret

@app.route('/api/private_event/feed', methods=['GET'])
@jwt_to_session_cookie
@login_required
def api_private_event_feed():
    ret = []
    for pe in ActualPrivateEvents().values():
        ret.append(clone_and_append_social(current_user.player_id, pe))
    if(request.headers['Accept'] == 'application/json'):
        return jsonify(ret)
    return jsonPrivateEventFeedToProtobuf(ret).SerializeToString(), 200

def jsonPrivateEventToProtobuf(je):
    ret = events_pb2.PrivateEventProto()
    ret.id = je['id']
    ret.sport = sport_from_str(je['sport'])
    ret.eventStart = stime_to_timestamp(je['eventStart'])*1000
    ret.routeId = je['routeId']
    ret.startLocation = je['startLocation']
    ret.durationInSeconds = je['durationInSeconds']
    ret.distanceInMeters = je['distanceInMeters']
    if 'description' in je:
        ret.description = je['description']
    ret.workoutHash = je['workoutHash']
    ret.organizerId = je['organizerProfileId']
    for jinv in je['eventInvites']:
        jp = jinv['invitedProfile']
        inv = ret.eventInvites.add()
        inv.profile.player_id = jp['id']
        inv.profile.firstName = jp['firstName']
        inv.profile.lastName = jp['lastName']
        if jp['imageSrc'] is not None:
            inv.profile.imageSrc = jp['imageSrc']
        inv.profile.enrolledZwiftAcademy = jp['enrolledZwiftAcademy']
        inv.profile.male = jp['male']
        inv.profile.player_type = profile_pb2.PlayerType.Value(jp['playerType'])
        inv.status = events_pb2.EventInviteStatus.Value(jinv['status'])
    ret.showResults = je['showResults']
    ret.laps = je['laps']
    ret.rubberbanding = je['rubberbanding']
    return ret

@app.route('/api/private_event/<int:event_id>', methods=['GET'])
@jwt_to_session_cookie
@login_required
def api_private_event_id(event_id):
    ret = clone_and_append_social(current_user.player_id, ActualPrivateEvents()[event_id])
    if(request.headers['Accept'] == 'application/json'):
        return jsonify(ret)
    return jsonPrivateEventToProtobuf(ret).SerializeToString(), 200

@app.route('/api/private_event/entitlement', methods=['GET'])
def api_private_event_entitlement():
    return jsonify({"entitled": True})

@app.route('/relay/events/subgroups/<int:meetup_id>/late-join', methods=['GET'])
@jwt_to_session_cookie
@login_required
def relay_events_subgroups_id_late_join(meetup_id):
    ape = ActualPrivateEvents()
    if meetup_id in ape.keys():
        event = jsonPrivateEventToProtobuf(ape[meetup_id])
        leader = None
        if event.organizerId in online and online[event.organizerId].groupId == meetup_id and event.organizerId != current_user.player_id:
            leader = event.organizerId
        else:
            for player_id in online.keys():
                if online[player_id].groupId == meetup_id and player_id != current_user.player_id:
                    leader = player_id
                    break
        if leader is not None:
            state = online[leader]
            lj = events_pb2.LateJoinInformation()
            lj.road_id = road_id(state)
            lj.road_time = state.roadTime / 1005000
            lj.is_forward = is_forward(state)
            lj.organizerId = leader
            lj.lj_f5 = 0
            lj.lj_f6 = 0
            lj.lj_f7 = 0
            return lj.SerializeToString(), 200
    return '', 200


def get_week_range(dt):
     d = (dt - datetime.timedelta(days = dt.weekday())).replace(hour=0, minute=0, second=0, microsecond=0)
     first = d
     last = d + datetime.timedelta(days=6, hours=23, minutes=59, seconds=59)
     return first, last

def get_month_range(dt):
     num_days = calendar.monthrange(dt.year, dt.month)[1]
     first = datetime.datetime(dt.year, dt.month, 1)
     last = datetime.datetime(dt.year, dt.month, num_days, 23, 59, 59)
     return first, last


def unix_time_millis(dt):
    return int(dt.timestamp()*1000)


def fill_in_goal_progress(goal, player_id):
    local_now = datetime.datetime.now()
    utc_offset = datetime.datetime.fromtimestamp(0) - datetime.datetime.utcfromtimestamp(0)
    if goal.periodicity == 0:  # weekly
        first_dt, last_dt = get_week_range(local_now)
    else:  # monthly
        first_dt, last_dt = get_month_range(local_now)

    common_sql = """FROM activity
                    WHERE player_id = :p AND sport = :s
                    AND strftime('%s', start_date) >= strftime('%s', :f)
                    AND strftime('%s', start_date) <= strftime('%s', :l)"""
    args = {"p": player_id, "s": goal.sport, "f": first_dt-utc_offset, "l": last_dt-utc_offset}
    if goal.type == goal_pb2.GoalType.DISTANCE:
        distance = db.session.execute(sqlalchemy.text('SELECT SUM(distanceInMeters) %s' % common_sql), args).first()[0]
        if distance:
            goal.actual_distance = distance
            goal.actual_duration = distance
        else:
            goal.actual_distance = 0.0
            goal.actual_duration = 0.0

    else:  # duration
        duration = db.session.execute(sqlalchemy.text('SELECT SUM(julianday(end_date)-julianday(start_date)) %s' % common_sql), args).first()[0]
        if duration:
            goal.actual_duration = duration*1440  # convert from days to minutes
            goal.actual_distance = duration*1440
        else:
            goal.actual_duration = 0.0
            goal.actual_distance = 0.0


def set_goal_end_date_now(goal):
    local_now = datetime.datetime.now()
    utc_offset = int((datetime.datetime.fromtimestamp(0) - datetime.datetime.utcfromtimestamp(0)).total_seconds())
    if goal.periodicity == 0:  # weekly
        goal.period_end_date = unix_time_millis(get_week_range(local_now)[1]) - utc_offset
    else:  # monthly
        goal.period_end_date = unix_time_millis(get_month_range(local_now)[1]) - utc_offset

def str_sport(int_sport):
    if int_sport == 1:
        return "RUNNING"
    return "CYCLING"

def sport_from_str(str_sport):
    if str_sport == 'CYCLING':
        return 0
    return 1 #running

def str_timestamp(ts):
    if ts == None:
        return None
    else:
        sec = int(ts/1000)
        ms = ts % 1000
        return datetime.datetime.utcfromtimestamp(sec).strftime('%Y-%m-%dT%H:%M:%S.') + str(ms).zfill(3) + "+0000"

def str_timestamp_json(ts):
    if ts == 0:
        return None
    else:
        return str_timestamp(ts)

def goalProtobufToJson(goal):
    return {"id":goal.id,"profileId":goal.player_id,"sport":str_sport(goal.sport),"name":goal.name,"type":int(goal.type),"periodicity":int(goal.periodicity),
"targetDistanceInMeters":goal.target_distance,"targetDurationInMinutes":goal.target_duration,"actualDistanceInMeters":goal.actual_distance,
"actualDurationInMinutes":goal.actual_duration,"createdOn":str_timestamp_json(goal.created_on),
"periodEndDate":str_timestamp_json(goal.period_end_date),"status":int(goal.status),"timezone":goal.timezone}

def goalJsonToProtobuf(json_goal):
    goal = goal_pb2.Goal()
    goal.sport = sport_from_str(json_goal['sport'])
    goal.id = json_goal['id']
    goal.name = json_goal['name']
    goal.periodicity = int(json_goal['periodicity'])
    goal.type = int(json_goal['type'])
    goal.status = goal_pb2.GoalStatus.ACTIVE
    goal.target_distance = json_goal['targetDistanceInMeters']
    goal.target_duration = json_goal['targetDurationInMinutes']
    goal.actual_distance = json_goal['actualDistanceInMeters']
    goal.actual_duration = json_goal['actualDurationInMinutes']
    goal.player_id = json_goal['profileId']
    return goal

@app.route('/api/profiles/<int:player_id>/goals/<int:goal_id>', methods=['PUT'])
@jwt_to_session_cookie
@login_required
def api_profiles_goals_put(player_id, goal_id):
    if player_id != current_user.player_id:
        return '', 401
    if not request.stream:
        return '', 400
    str_goal = request.stream.read()
    json_goal = json.loads(str_goal)
    goal = goalJsonToProtobuf(json_goal)
    update_protobuf_in_db(Goal, goal, goal.id)
    return jsonify(json_goal)

def select_protobuf_goals(player_id, limit):
    goals = goal_pb2.Goals()
    if limit > 0:
        stmt = sqlalchemy.text("SELECT * FROM goal WHERE player_id = :p LIMIT :l")
        rows = db.session.execute(stmt, {"p": player_id, "l": limit})
        need_update = list()
        for row in rows:
            goal = goals.goals.add()
            row_to_protobuf(row, goal)
            end_dt = datetime.datetime.fromtimestamp(goal.period_end_date / 1000)
            if end_dt < datetime.datetime.utcnow():
                need_update.append(goal)
            fill_in_goal_progress(goal, player_id)
        for goal in need_update:
            set_goal_end_date_now(goal)
            update_protobuf_in_db(Goal, goal, goal.id)
    return goals

def convert_goals_to_json(goals):
    json_goals = []
    for goal in goals.goals:
        json_goal = goalProtobufToJson(goal)
        json_goals.append(json_goal)
    return json_goals

@app.route('/api/profiles/<int:player_id>/goals', methods=['GET', 'POST'])
@jwt_to_session_cookie
@login_required
def api_profiles_goals(player_id):
    if player_id != current_user.player_id:
        return '', 401
    if request.method == 'POST':
        if not request.stream:
            return '', 400
        if(request.headers['Content-Type'] == 'application/x-protobuf-lite'):
            goal = goal_pb2.Goal()
            goal.ParseFromString(request.stream.read())
        else:
            str_goal = request.stream.read()
            json_goal = json.loads(str_goal)
            goal = goalJsonToProtobuf(json_goal)
        goal.created_on = unix_time_millis(datetime.datetime.utcnow())
        set_goal_end_date_now(goal)
        fill_in_goal_progress(goal, player_id)
        goal.id = insert_protobuf_into_db(Goal, goal)

        if request.headers['Accept'] == 'application/json':
            return jsonify(goalProtobufToJson(goal))
        else:
            return goal.SerializeToString(), 200

    # request.method == 'GET'
    goals = select_protobuf_goals(player_id, 100)

    if request.headers['Accept'] == 'application/json':
        json_goals = convert_goals_to_json(goals)
        return jsonify(json_goals) # json for ZCA
    else:
        return goals.SerializeToString(), 200 # protobuf for ZG


@app.route('/api/profiles/<int:player_id>/goals/<int:goal_id>', methods=['DELETE'])
@jwt_to_session_cookie
@login_required
def api_profiles_goals_id(player_id, goal_id):
    if player_id != current_user.player_id:
        return '', 401
    db.session.execute(sqlalchemy.text("DELETE FROM goal WHERE id = :i"), {"i": goal_id})
    db.session.commit()
    return '', 200


@app.route('/api/tcp-config', methods=['GET'])
@app.route('/relay/tcp-config', methods=['GET'])
def api_tcp_config():
    infos = per_session_info_pb2.TcpConfig()
    info = infos.nodes.add()
    if request.remote_addr == '127.0.0.1':  # to avoid needing hairpinning
        info.ip = "127.0.0.1"
    else:
        info.ip = server_ip
    info.port = 3023
    return infos.SerializeToString(), 200


def add_player_to_world(player, course_world, is_pace_partner=False, is_bot=False):
    course_id = get_course(player)
    if course_id in course_world.keys():
        partial_profile = get_partial_profile(player.id)
        online_player = None
        if is_pace_partner:
            online_player = course_world[course_id].pacer_bots.add()
            online_player.route = partial_profile.route
            if player.sport == profile_pb2.Sport.CYCLING:
                online_player.ride_power = player.power
            else:
                online_player.speed = player.speed
        elif is_bot:
            online_player = course_world[course_id].others.add()
        else: # to be able to join zwifter using new home screen
            online_player = course_world[course_id].followees.add()
        online_player.id = player.id
        online_player.firstName = partial_profile.first_name
        online_player.lastName = partial_profile.last_name
        online_player.distance = player.distance
        online_player.time = player.time
        online_player.country_code = partial_profile.country_code
        online_player.sport = player.sport
        online_player.power = player.power
        online_player.x = player.x
        online_player.y_altitude = player.y_altitude
        online_player.z = player.z
        course_world[course_id].zwifters += 1


def relay_worlds_generic(server_realm=None):
    # Android client also requests a JSON version
    if request.headers['Accept'] == 'application/json':
        world = { 'currentDateTime': int(get_utc_time()),
                  'currentWorldTime': world_time(),
                  'friendsInWorld': [],
                  'mapId': 1, #maybe, 13 for watopia?
                  'name': 'Public Watopia',
                  'playerCount': 0,
                  'worldId': udp_node_msgs_pb2.ZofflineConstants.RealmID
                }
        if server_realm:
            world['worldId'] = server_realm
            return jsonify(world)
        else:
            return jsonify([ world ])
    else:  # protobuf request
        worlds = world_pb2.DropInWorldList()
        world = None
        course_world = {}
        for course in courses_lookup.keys():
            world = worlds.worlds.add()
            world.id = udp_node_msgs_pb2.ZofflineConstants.RealmID
            world.name = 'Public Watopia'
            world.course_id = course
            world.world_time = world_time()
            world.real_time = int(get_time())
            world.zwifters = 0
            course_world[course] = world
        for p_id in online.keys():
            player = online[p_id]
            add_player_to_world(player, course_world)
        for p_id in global_pace_partners.keys():
            pace_partner_variables = global_pace_partners[p_id]
            pace_partner = pace_partner_variables.route.states[pace_partner_variables.position]
            add_player_to_world(pace_partner, course_world, is_pace_partner=True)
        for p_id in global_bots.keys():
            bot_variables = global_bots[p_id]
            bot = bot_variables.route.states[bot_variables.position]
            add_player_to_world(bot, course_world, is_bot=True)
        if server_realm:
            world.id = server_realm
            return world.SerializeToString()
        else:
            return worlds.SerializeToString()


@app.route('/relay/worlds', methods=['GET'])
@app.route('/relay/dropin', methods=['GET']) #zwift::protobuf::DropInWorldList
def relay_worlds():
    return relay_worlds_generic()

def iterableToJson(it):
    if it == None:
        return None
    ret = []
    for i in it:
        ret.append(i)
    return ret

def convert_event_to_json(event):
    esgs = []
    for event_cat in event.category:
        esgs.append({"id":event_cat.id,"name":event_cat.name,"description":event_cat.description,"label":event_cat.label, \
"subgroupLabel":event_cat.name[-1],"rulesId":event_cat.rules_id,"mapId":event_cat.course_id,"routeId":event_cat.route_id,"routeUrl":event_cat.routeUrl, \
"jerseyHash":event_cat.jerseyHash,"bikeHash":event_cat.bikeHash,"startLocation":event_cat.startLocation,"invitedLeaders":iterableToJson(event_cat.invitedLeaders), \
"invitedSweepers":iterableToJson(event_cat.invitedSweepers),"paceType":event_cat.paceType,"fromPaceValue":event_cat.fromPaceValue,"toPaceValue":event_cat.toPaceValue, \
"fieldLimit":None,"registrationStart":str_timestamp_json(event_cat.registrationStart),"registrationEnd":str_timestamp_json(event_cat.registrationEnd),"lineUpStart":str_timestamp_json(event_cat.lineUpStart), \
"lineUpEnd":str_timestamp_json(event_cat.lineUpEnd),"eventSubgroupStart":str_timestamp_json(event_cat.eventSubgroupStart),"durationInSeconds":event_cat.durationInSeconds,"laps":event_cat.laps, \
"distanceInMeters":event_cat.distanceInMeters,"signedUp":False,"signupStatus":1,"registered":False,"registrationStatus":1,"followeeEntrantCount":0, \
"totalEntrantCount":0,"followeeSignedUpCount":0,"totalSignedUpCount":0,"followeeJoinedCount":0,"totalJoinedCount":0,"auxiliaryUrl":"", \
"rulesSet":["ALLOWS_LATE_JOIN"],"workoutHash":None,"customUrl":event_cat.customUrl,"overrideMapPreferences":False, \
"tags":[""],"lateJoinInMinutes":event_cat.lateJoinInMinutes,"timeTrialOptions":None,"qualificationRuleIds":None,"accessValidationResult":None})
    return {"id":event.id,"worldId":event.server_realm,"name":event.name,"description":event.description,"shortName":None,"mapId":event.course_id, \
"shortDescription":None,"imageUrl":event.imageUrl,"routeId":event.route_id,"rulesId":event.rules_id,"rulesSet":["ALLOWS_LATE_JOIN"], \
"routeUrl":None,"jerseyHash":event.jerseyHash,"bikeHash":None,"visible":event.visible,"overrideMapPreferences":event.overrideMapPreferences,"eventStart":str_timestamp_json(event.eventStart), "tags":[""], \
"durationInSeconds":event.durationInSeconds,"distanceInMeters":event.distanceInMeters,"laps":event.laps,"privateEvent":False,"invisibleToNonParticipants":event.invisibleToNonParticipants, \
"followeeEntrantCount":0,"totalEntrantCount":0,"followeeSignedUpCount":0,"totalSignedUpCount":0,"followeeJoinedCount":0,"totalJoinedCount":0, \
"eventSeries":None,"auxiliaryUrl":"","imageS3Name":None,"imageS3Bucket":None,"sport":str_sport(event.sport),"cullingType":"CULLING_EVERYBODY", \
"recurring":True,"recurringOffset":None,"publishRecurring":True,"parentId":None,"type":events_pb2._EVENTTYPEV2.values_by_number[int(event.eventType)].name, \
"eventType":events_pb2._EVENTTYPE.values_by_number[int(event.eventType)].name, \
"workoutHash":None,"customUrl":"","restricted":False,"unlisted":False,"eventSecret":None,"accessExpression":None,"qualificationRuleIds":None, \
"lateJoinInMinutes":event.lateJoinInMinutes,"timeTrialOptions":None,"microserviceName":None,"microserviceExternalResourceId":None, \
"microserviceEventVisibility":None, "minGameVersion":None,"recordable":True,"imported":False,"eventTemplateId":None, "eventSubgroups": esgs }

def convert_events_to_json(events):
    json_events = []
    for e in events.events:
        json_event = convert_event_to_json(e)
        json_events.append(json_event)
    return json_events

def transformPrivateEvents(player_id, max_count, status):
    ret = []
    if max_count > 0:
        for e in ActualPrivateEvents().values():
            for i in e['eventInvites']:
                if i['invitedProfile']['id'] == player_id:
                    if i['status'] == status:
                        e_clone = deepcopy(e)
                        e_clone['inviteStatus'] = status
                        ret.append(e_clone)
                        if len(ret) >= max_count:
                            return ret
    return ret

#todo: followingCount=3&playerSport=all&eventSport=CYCLING&fetchCampaign=true
@app.route('/relay/worlds/<int:server_realm>/aggregate/mobile', methods=['GET'])
@jwt_to_session_cookie
@login_required
def relay_worlds_id_aggregate_mobile(server_realm):
    goalCount = int(request.args.get('goalCount'))
    goals = select_protobuf_goals(current_user.player_id, goalCount)
    json_goals = convert_goals_to_json(goals)
    activityCount = int(request.args.get('activityCount'))
    json_activities = select_activities_json(current_user.player_id, activityCount)
    eventCount = int(request.args.get('eventCount'))
    events = get_events(eventCount, 'CYCLING') #runners, sorry!
    json_events = convert_events_to_json(events)
    pendingEventInviteCount = int(request.args.get('pendingEventInviteCount'))
    ppeFeed = transformPrivateEvents(current_user.player_id, pendingEventInviteCount, 'PENDING')
    acceptedEventInviteCount = int(request.args.get('acceptedEventInviteCount'))
    apeFeed = transformPrivateEvents(current_user.player_id, acceptedEventInviteCount, 'ACCEPTED')
    return jsonify({"events":json_events,"goals":json_goals,"activities":json_activities,"pendingPrivateEventFeed":ppeFeed,"acceptedPrivateEventFeed":apeFeed,"hasFolloweesToRideOn":False, \
    "worldName":"MAKURIISLANDS","playerCount": len(online),"followingPlayerCount":0,"followingPlayers":[]})

@app.route('/relay/worlds/<int:server_realm>', methods=['GET'], strict_slashes=False)
def relay_worlds_id(server_realm):
    return relay_worlds_generic(server_realm)


@app.route('/relay/worlds/<int:server_realm>/join', methods=['POST'])
def relay_worlds_id_join(server_realm):
    return '{"worldTime":%ld}' % world_time()


@app.route('/relay/worlds/<int:server_realm>/players/<int:player_id>', methods=['GET'])
def relay_worlds_id_players_id(server_realm, player_id):
    if player_id in online.keys():
        player = online[player_id]
        return player.SerializeToString()
    if player_id in global_pace_partners.keys():
        pace_partner = global_pace_partners[player_id]
        state = pace_partner.route.states[pace_partner.position]
        state.world = get_course(state)
        state.route = get_partial_profile(player_id).route
        return state.SerializeToString()
    if player_id in global_bots.keys():
        bot = global_bots[player_id]
        return bot.route.states[bot.position].SerializeToString()
    return ""


@app.route('/relay/worlds/hash-seeds', methods=['GET'])
def relay_worlds_hash_seeds():
    seeds = hash_seeds_pb2.HashSeeds()
    for x in range(4):
        seed = seeds.seeds.add()
        seed.seed1 = int(random.getrandbits(31))
        seed.seed2 = int(random.getrandbits(31))
        seed.expiryDate = world_time()+(10800+x*1200)*1000
    return seeds.SerializeToString(), 200


@app.route('/relay/worlds/attributes', methods=['POST'])
@jwt_to_session_cookie
@login_required
def relay_worlds_attributes():
    player_update = udp_node_msgs_pb2.WorldAttribute()
    player_update.ParseFromString(request.stream.read())
    player_update.world_time_expire = world_time() + 60000
    player_update.wa_f12 = 1
    player_update.timestamp = int(get_utc_time() * 1000000)
    for receiving_player_id in online.keys():
        should_receive = False
        if player_update.wa_type in [udp_node_msgs_pb2.WA_TYPE.WAT_SPA, udp_node_msgs_pb2.WA_TYPE.WAT_SR]:
            receiving_player = online[receiving_player_id]
            # Chat message
            if player_update.wa_type == udp_node_msgs_pb2.WA_TYPE.WAT_SPA:
                chat_message = tcp_node_msgs_pb2.SocialPlayerAction()
                chat_message.ParseFromString(player_update.payload)
                if chat_message.message == '/regroup':
                    regroup_ghosts(chat_message.player_id)
                    return '', 201
                sending_player_id = chat_message.player_id
                if sending_player_id in online:
                    sending_player = online[sending_player_id]
                    if is_nearby(sending_player, receiving_player):
                        should_receive = True
            # Segment complete
            else:
                segment_complete = segment_result_pb2.SegmentResult()
                segment_complete.ParseFromString(player_update.payload)
                sending_player_id = segment_complete.player_id
                if sending_player_id in online and receiving_player_id != sending_player_id:
                    sending_player = online[sending_player_id]
                    if get_course(sending_player) == get_course(receiving_player) or receiving_player.watchingRiderId == sending_player_id:
                        should_receive = True
        # Other PlayerUpdate, send to all
        else:
            should_receive = True
        if should_receive:
            enqueue_player_update(receiving_player_id, player_update.SerializeToString())
    # If it's a chat message, send to Discord
    if player_update.wa_type == udp_node_msgs_pb2.WA_TYPE.WAT_SPA:
        chat_message = tcp_node_msgs_pb2.SocialPlayerAction()
        chat_message.ParseFromString(player_update.payload)
        discord.send_message(chat_message.message, chat_message.player_id)
    return '', 201


@app.route('/api/segment-results', methods=['POST'])
@jwt_to_session_cookie
@login_required
def api_segment_results():
    if not request.stream:
        return '', 400
    data = request.stream.read()
    result = segment_result_pb2.SegmentResult()
    result.ParseFromString(data)
    if result.segment_id == 1:
        return '', 400
    result.world_time = world_time()
    result.finish_time_str = datetime.datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
    result.sport = 0
    result.id = insert_protobuf_into_db(SegmentResult, result)

    # Previously done in /relay/worlds/attributes
    player_update = udp_node_msgs_pb2.WorldAttribute()
    player_update.server_realm = udp_node_msgs_pb2.ZofflineConstants.RealmID
    player_update.wa_type = udp_node_msgs_pb2.WA_TYPE.WAT_SR
    player_update.payload = data
    player_update.world_time_born = world_time()
    player_update.world_time_expire = world_time() + 60000
    player_update.timestamp = int(get_utc_time() * 1000000)
    sending_player_id = result.player_id
    if sending_player_id in online:
        sending_player = online[sending_player_id]
        for receiving_player_id in online.keys():
            if receiving_player_id != sending_player_id:
                receiving_player = online[receiving_player_id]
                if get_course(sending_player) == get_course(receiving_player) or receiving_player.watchingRiderId == sending_player_id:
                    enqueue_player_update(receiving_player_id, player_update.SerializeToString())

    return {"id": result.id}


@app.route('/api/personal-records/my-records', methods=['GET'])
@jwt_to_session_cookie
@login_required
def api_personal_records_my_records():
    if not request.args.get('segmentId'):
        return '', 422
    segment_id = int(request.args.get('segmentId'))
    from_date = request.args.get('from')
    to_date = request.args.get('to')

    results = segment_result_pb2.SegmentResults()
    results.server_realm = udp_node_msgs_pb2.ZofflineConstants.RealmID
    results.segment_id = segment_id

    where_stmt = "WHERE segment_id = :s AND player_id = :p"
    args = {"s": segment_id, "p": current_user.player_id}
    if from_date:
        where_stmt += " AND strftime('%s', finish_time_str) > strftime('%s', :f)"
        args.update({"f": from_date})
    if to_date:
        where_stmt += " AND strftime('%s', finish_time_str) < strftime('%s', :t)"
        args.update({"t": to_date})
    rows = db.session.execute(sqlalchemy.text("SELECT * FROM segment_result %s" % where_stmt), args)
    for row in rows:
        result = results.segment_results.add()
        row_to_protobuf(row, result, ['server_realm', 'course_id', 'segment_id', 'event_subgroup_id', 'finish_time_str', 'f14', 'time', 'player_type', 'f22', 'f23'])

    return results.SerializeToString(), 200


@app.route('/live-segment-results-service/leaders', methods=['GET'])
def live_segment_results_service_leaders():
    results = segment_result_pb2.SegmentResults()
    results.server_realm = 0
    results.segment_id = 0
    stmt = sqlalchemy.text("""SELECT s1.* FROM segment_result s1
        JOIN (SELECT s.player_id, s.segment_id, MIN(s.elapsed_ms) AS min_time
            FROM segment_result s WHERE world_time > :w GROUP BY s.player_id, s.segment_id) s2
            ON s2.player_id = s1.player_id AND s2.min_time = s1.elapsed_ms
        GROUP BY s1.player_id, s1.elapsed_ms ORDER BY s1.segment_id, s1.elapsed_ms LIMIT 1000""")
    rows = db.session.execute(stmt, {"w": world_time()-60*60*1000})
    for row in rows:
        result = results.segment_results.add()
        row_to_protobuf(row, result, ['f14', 'time', 'player_type', 'f22'])
    return results.SerializeToString(), 200


@app.route('/live-segment-results-service/leaderboard/<segment_id>', methods=['GET'])
def live_segment_results_service_leaderboard_segment_id(segment_id):
    segment_id = int(segment_id)
    results = segment_result_pb2.SegmentResults()
    results.server_realm = 0
    results.segment_id = segment_id
    stmt = sqlalchemy.text("""SELECT s1.* FROM segment_result s1
        JOIN (SELECT s.player_id, MIN(s.elapsed_ms) AS min_time
            FROM segment_result s WHERE segment_id = :s AND world_time > :w GROUP BY s.player_id) s2
            ON s2.player_id = s1.player_id AND s2.min_time = s1.elapsed_ms
        GROUP BY s1.player_id, s1.elapsed_ms ORDER BY s1.elapsed_ms LIMIT 1000""")
    rows = db.session.execute(stmt, {"s": segment_id, "w": world_time()-60*60*1000})
    for row in rows:
        result = results.segment_results.add()
        row_to_protobuf(row, result, ['f14', 'time', 'player_type', 'f22'])
    return results.SerializeToString(), 200


@app.route('/relay/worlds/<int:server_realm>/leave', methods=['POST'])
def relay_worlds_leave(server_realm):
    return '{"worldtime":%ld}' % world_time()


@app.route('/experimentation/v1/variant', methods=['POST'])
@jwt_to_session_cookie
@login_required
def experimentation_v1_variant():
    stream = variants_pb2.FeatureResponse()
    stream.ParseFromString(request.stream.read())
    variants = {}
    with open(os.path.join(SCRIPT_DIR, "variants.txt")) as f:
        vs = variants_pb2.FeatureResponse()
        Parse(f.read(), vs)
        for v in vs.variants:
            variants[v.name] = v
    variants['game_1_20_home_screen'].value = current_user.new_home
    response = variants_pb2.FeatureResponse()
    for req in stream.variants:
        if req.name in variants:
            response.variants.append(variants[req.name])
        else:
            logger.info("Unknown feature: " + req.name)
    return response.SerializeToString(), 200

def get_profile_saved_game_achiev2_40_bytes():
    profile_file = '%s/%s/profile.bin' % (STORAGE_DIR, current_user.player_id)
    if not os.path.isfile(profile_file):
        return b''
    with open(profile_file, 'rb') as fd:
        profile = profile_pb2.PlayerProfile()
        profile.ParseFromString(fd.read())
        if len(profile.saved_game) > 0x150 and profile.saved_game[0x108] == 2: #checking 2 from 0x10000002: achiev_badges2_40
            return profile.saved_game[0x110:0x110+0x40] #0x110 = accessories1_100 + 2x8-byte headers
        else:
            return b''

@app.route('/api/achievement/loadPlayerAchievements', methods=['GET'])
@jwt_to_session_cookie
@login_required
def achievement_loadPlayerAchievements():
    achievements_file = os.path.join(STORAGE_DIR, str(current_user.player_id), 'achievements.bin')
    if not os.path.isfile(achievements_file):
        converted = profile_pb2.Achievements()
        old_achiev_bits = get_profile_saved_game_achiev2_40_bytes()
        for ach_id in range(8 * len(old_achiev_bits)):
            if (old_achiev_bits[ach_id // 8] >> (ach_id % 8)) & 0x1:
                converted.achievements.add().id = ach_id
        with open(achievements_file, 'wb') as f:
            f.write(converted.SerializeToString())
    with open(achievements_file, 'rb') as f:
        return f.read(), 200

@app.route('/api/achievement/unlock', methods=['POST'])
@jwt_to_session_cookie
@login_required
def achievement_unlock():
    if not request.stream:
        return '', 400
    with open(os.path.join(STORAGE_DIR, str(current_user.player_id), 'achievements.bin'), 'wb') as f:
        f.write(request.stream.read())
    return '', 202

# if we respond to this request with an empty json a "tutorial" will be presented in ZCA
# and for each completed step it will POST /api/achievement/unlock/<id>
@app.route('/api/achievement/category/<category_id>', methods=['GET'])
def api_achievement_category(category_id):
    return '', 404 # returning error for now, since some steps can't be completed


@app.teardown_request
def teardown_request(exception):
    db.session.close()
    if exception != None:
        print('Exception: %s' % exception)


def move_old_profile():
    # Before multi profile support only a single profile located in storage
    # named profile.bin existed. If upgrading from this, convert to
    # multi profile file structure.
    profile_file = '%s/profile.bin' % STORAGE_DIR
    if os.path.isfile(profile_file):
        with open(profile_file, 'rb') as fd:
            profile = profile_pb2.PlayerProfile()
            profile.ParseFromString(fd.read())
            profile_dir = '%s/%s' % (STORAGE_DIR, profile.id)
            try:
                if not os.path.isdir(profile_dir):
                    os.makedirs(profile_dir)
            except IOError as e:
                logger.error("failed to create profile dir (%s):  %s", profile_dir, str(e))
                sys.exit(1)
        os.rename(profile_file, '%s/profile.bin' % profile_dir)
        strava_file = '%s/strava_token.txt' % STORAGE_DIR
        if os.path.isfile(strava_file):
            os.rename(strava_file, '%s/strava_token.txt' % profile_dir)

def save_fit(player_id, name, data):
    fit_dir = os.path.join(STORAGE_DIR, str(player_id), 'fit')
    try:
        if not os.path.isdir(fit_dir):
            os.makedirs(fit_dir)
    except IOError as e:
        logger.error("failed to create fit dir (%s):  %s", fit_dir, str(e))
        return
    with open(os.path.join(fit_dir, name), 'wb') as f:
        f.write(data)

def migrate_database():
    # Migrate database if necessary
    if not os.access(DATABASE_PATH, os.W_OK):
        logging.error("zwift-offline.db is not writable. Unable to upgrade database!")
        return
    row = Version.query.first()
    if not row:
        db.session.add(Version(version=DATABASE_CUR_VER))
        db.session.commit()
        return
    version = row.version
    if version != 2:
        return
    # Database needs to be upgraded, try to back it up first
    try:  # Try writing to storage dir
        copyfile(DATABASE_PATH, "%s.v%d.%d.bak" % (DATABASE_PATH, version, int(get_utc_time())))
    except:
        try:  # Fall back to a temporary dir
            copyfile(DATABASE_PATH, "%s/zwift-offline.db.v%s.%d.bak" % (tempfile.gettempdir(), version, int(get_utc_time())))
        except Exception as exc:
            logging.warn("Failed to create a zoffline database backup prior to upgrading it. %s" % repr(exc))

    logging.warn("Migrating database, please wait")
    db.session.execute('ALTER TABLE activity RENAME TO activity_old')
    db.session.execute('ALTER TABLE goal RENAME TO goal_old')
    db.session.execute('ALTER TABLE segment_result RENAME TO segment_result_old')
    db.session.execute('ALTER TABLE playback RENAME TO playback_old')
    db.create_all()

    import ast
    # Select every column except 'id' and cast 'fit' as hex - after 77ff84e fit data was stored incorrectly
    rows = db.session.execute('SELECT player_id, f3, name, f5, f6, start_date, end_date, distance, avg_heart_rate, max_heart_rate, avg_watts, max_watts, avg_cadence, max_cadence, avg_speed, max_speed, calories, total_elevation, strava_upload_id, strava_activity_id, f23, hex(fit), fit_filename, f29, date FROM activity_old')
    for row in rows:
        d = {k: row[k] for k in row.keys()}
        d['player_id'] = int(d['player_id'])
        d['course_id'] = d.pop('f3')
        d['privateActivity'] = d.pop('f6')
        d['distanceInMeters'] = d.pop('distance')
        d['sport'] = d.pop('f29')
        fit_data = bytes.fromhex(d['hex(fit)'])
        if fit_data[0:2] == b"b'":
            try:
                fit_data = ast.literal_eval(fit_data.decode("ascii"))
            except:
                d['fit_filename'] = 'corrupted'
        del d['hex(fit)']
        orm_act = Activity(**d)
        db.session.add(orm_act)
        db.session.flush()
        fit_filename = '%s - %s' % (orm_act.id, d['fit_filename'])
        save_fit(d['player_id'], fit_filename, fit_data)

    rows = db.session.execute('SELECT * FROM goal_old')
    for row in rows:
        d = {k: row[k] for k in row.keys()}
        del d['id']
        d['player_id'] = int(d['player_id'])
        d['sport'] = d.pop('f3')
        d['created_on'] = int(d['created_on'])
        d['period_end_date'] = int(d['period_end_date'])
        d['status'] = int(d.pop('f13'))
        db.session.add(Goal(**d))

    rows = db.session.execute('SELECT * FROM segment_result_old')
    for row in rows:
        d = {k: row[k] for k in row.keys()}
        del d['id']
        d['player_id'] = int(d['player_id'])
        d['server_realm'] = d.pop('f3')
        d['course_id'] = d.pop('f4')
        d['segment_id'] = toSigned(int(d['segment_id']), 8)
        d['event_subgroup_id'] = int(d['event_subgroup_id'])
        d['world_time'] = int(d['world_time'])
        d['elapsed_ms'] = int(d['elapsed_ms'])
        d['power_source_model'] = d.pop('f12')
        d['weight_in_grams'] = d.pop('f13')
        d['avg_power'] = d.pop('f15')
        d['is_male'] = d.pop('f16')
        d['time'] = d.pop('f17')
        d['player_type'] = d.pop('f18')
        d['avg_hr'] = d.pop('f19')
        d['sport'] = d.pop('f20')
        db.session.add(SegmentResult(**d))

    rows = db.session.execute('SELECT * FROM playback_old')
    for row in rows:
        d = {k: row[k] for k in row.keys()}
        d['segment_id'] = toSigned(int(d['segment_id']), 8)
        db.session.add(Playback(**d))

    db.session.execute('DROP TABLE activity_old')
    db.session.execute('DROP TABLE goal_old')
    db.session.execute('DROP TABLE segment_result_old')
    db.session.execute('DROP TABLE playback_old')

    Version.query.filter_by(version=2).update(dict(version=DATABASE_CUR_VER))
    db.session.commit()
    db.session.execute('vacuum') #shrink database
    logging.warn("Database migration completed")


def check_columns(table_class, table_name):
    rows = db.session.execute("PRAGMA table_info(%s)" % table_name)
    should_have_columns = table_class.metadata.tables[table_name].columns
    current_columns = list()
    for row in rows:
        current_columns.append(row[1])
    for column in should_have_columns:
        if not column.name in current_columns:
            nulltext = None
            if column.nullable:
                nulltext = "NULL"
            else:
                nulltext = "NOT NULL"
            defaulttext = None
            if column.default == None:
                defaulttext = ""
            else:
                defaulttext = " DEFAULT %s" % column.default.arg
            db.session.execute("ALTER TABLE %s ADD %s %s %s%s" % (table_name, column.name, column.type, nulltext, defaulttext))
            db.session.commit()


def send_server_back_online_message():
    time.sleep(30)
    message = "We're back online. Ride on!"
    send_message_to_all_online(message)
    discord.send_message(message)


@app.before_first_request
def before_first_request():
    move_old_profile()
    db.create_all()
    db.session.commit()
    check_columns(User, 'user')
    migrate_database()
    db.session.close()


####################
#
# Auth server (secure.zwift.com) routes below here
#
####################

@app.route('/auth/rb_bf03269xbi', methods=['POST'])
def auth_rb():
    return 'OK(Java)'


@app.route('/launcher', methods=['GET'])
@app.route('/launcher/realms/zwift/protocol/openid-connect/auth', methods=['GET'])
@app.route('/launcher/realms/zwift/protocol/openid-connect/registrations', methods=['GET'])
@app.route('/auth/realms/zwift/protocol/openid-connect/auth', methods=['GET'])
@app.route('/auth/realms/zwift/login-actions/request/login', methods=['GET', 'POST'])
@app.route('/auth/realms/zwift/protocol/openid-connect/registrations', methods=['GET'])
@app.route('/auth/realms/zwift/login-actions/startriding', methods=['GET'])  # Unused as it's a direct redirect now from auth/login
@app.route('/auth/realms/zwift/tokens/login', methods=['GET'])  # Called by Mac, but not Windows
@app.route('/auth/realms/zwift/tokens/registrations', methods=['GET'])  # Called by Mac, but not Windows
@app.route('/ride', methods=['GET'])
def launch_zwift():
    # Zwift client has switched to calling https://launcher.zwift.com/launcher/ride
    if request.path != "/ride" and not os.path.exists(AUTOLAUNCH_FILE):
        if MULTIPLAYER:
            return redirect(url_for('login'))
        else:
            return render_template("user_home.html", username="", enable_ghosts=os.path.exists(ENABLEGHOSTS_FILE), new_home=os.path.exists(NEWHOME_FILE), online=get_online(),
                is_admin=False, restarting=restarting, restarting_in_minutes=restarting_in_minutes)
    else:
        if MULTIPLAYER:
            return redirect("http://zwift/?code=zwift_refresh_token%s" % fake_refresh_token_with_session_cookie(request.cookies.get('remember_token')), 302)
        else:
            return redirect("http://zwift/?code=zwift_refresh_token%s" % REFRESH_TOKEN, 302)


def fake_refresh_token_with_session_cookie(session_cookie):
    refresh_token = jwt.decode(REFRESH_TOKEN, options=({'verify_signature': False, 'verify_aud': False}))
    refresh_token['session_cookie'] = session_cookie
    refresh_token = jwt.encode(refresh_token, 'nosecret')
    return refresh_token


def fake_jwt_with_session_cookie(session_cookie):
    access_token = jwt.decode(ACCESS_TOKEN, options=({'verify_signature': False, 'verify_aud': False}))
    access_token['session_cookie'] = session_cookie
    access_token = jwt.encode(access_token, 'nosecret')

    refresh_token = fake_refresh_token_with_session_cookie(session_cookie)

    return {"access_token":access_token,"expires_in":1000021600,"refresh_expires_in":611975560,"refresh_token":refresh_token,"token_type":"bearer","id_token":ID_TOKEN,"not-before-policy":1408478984,"session_state":"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3","scope":""}


@app.route('/auth/realms/zwift/protocol/openid-connect/token', methods=['POST'])
def auth_realms_zwift_protocol_openid_connect_token():
    # Android client login
    username = request.form.get('username')
    password = request.form.get('password')

    if username and MULTIPLAYER:
        user = User.query.filter_by(username=username).first()

        if user and check_password_hash(user.pass_hash, password):
            login_user(user, remember=True)
        else:
            return '', 401

    if MULTIPLAYER:
        # This is called once with ?code= in URL and once again with the refresh token
        if "code" in request.form:
            # Original code argument is replaced with session cookie from launcher
            refresh_token = jwt.decode(request.form['code'][19:], options=({'verify_signature': False, 'verify_aud': False}))
            session_cookie = refresh_token['session_cookie']
            return jsonify(fake_jwt_with_session_cookie(session_cookie)), 200
        elif "refresh_token" in request.form:
            token = jwt.decode(request.form['refresh_token'], options=({'verify_signature': False, 'verify_aud': False}))
            if 'session_cookie' in token:
                return jsonify(fake_jwt_with_session_cookie(token['session_cookie']))
            else:
                return '', 401
        else:  # android login
            current_user.enable_ghosts = user.enable_ghosts
            ghosts_enabled[current_user.player_id] = current_user.enable_ghosts
            current_user.new_home = user.new_home
            from flask_login import encode_cookie
            # cookie is not set in request since we just logged in so create it.
            return jsonify(fake_jwt_with_session_cookie(encode_cookie(str(session['_user_id'])))), 200
    else:
        AnonUser.enable_ghosts = os.path.exists(ENABLEGHOSTS_FILE)
        AnonUser.new_home = os.path.exists(NEWHOME_FILE)
        r = make_response(FAKE_JWT)
        r.mimetype = 'application/json'
        return r

@app.route('/auth/realms/zwift/protocol/openid-connect/logout', methods=['POST'])
def auth_realms_zwift_protocol_openid_connect_logout():
    # This is called on ZCA logout, we don't want the game client to logout (anyway jwt.decode would fail)
    session.clear()
    return '', 204

def save_option(option, file):
    if option:
        if not os.path.exists(file):
            f = open(file, 'w')
            f.close()
    elif os.path.exists(file):
        os.remove(file)

@app.route("/start-zwift" , methods=['POST'])
@login_required
def start_zwift():
    current_user.enable_ghosts = 'enableghosts' in request.form.keys()
    ghosts_enabled[current_user.player_id] = current_user.enable_ghosts
    current_user.new_home = 'newhome' in request.form.keys()
    if not MULTIPLAYER:
        save_option(current_user.enable_ghosts, ENABLEGHOSTS_FILE)
        save_option(current_user.new_home, NEWHOME_FILE)
    db.session.commit()
    selected_map = request.form['map']
    if selected_map == 'CALENDAR':
        return redirect("/ride", 302)
    else:
        response = make_response(redirect("http://cdn.zwift.com/map_override", 302))
        response.set_cookie('selected_map', selected_map, domain=".zwift.com")
        if MULTIPLAYER:
            response.set_cookie('remember_token', request.cookies['remember_token'], domain=".zwift.com")
        return response


def run_standalone(passed_online, passed_global_relay, passed_global_pace_partners, passed_global_bots, passed_global_ghosts, passed_ghosts_enabled, passed_save_ghost, passed_regroup_ghosts, passed_player_update_queue, passed_discord):
    global online
    global global_relay
    global global_pace_partners
    global global_bots
    global global_ghosts
    global ghosts_enabled
    global save_ghost
    global regroup_ghosts
    global player_update_queue
    global discord
    global login_manager
    online = passed_online
    global_relay = passed_global_relay
    global_pace_partners = passed_global_pace_partners
    global_bots = passed_global_bots
    global_ghosts = passed_global_ghosts
    ghosts_enabled = passed_ghosts_enabled
    save_ghost = passed_save_ghost
    regroup_ghosts = passed_regroup_ghosts
    player_update_queue = passed_player_update_queue
    discord = passed_discord
    login_manager = LoginManager()
    login_manager.login_view = 'login'
    login_manager.session_protection = None
    if not MULTIPLAYER:
        # Find first profile.bin if one exists and use it. Multi-profile
        # support is deprecated and now unsupported for non-multiplayer mode.
        player_id = None
        for name in os.listdir(STORAGE_DIR):
            path = "%s/%s" % (STORAGE_DIR, name)
            if os.path.isdir(path) and os.path.exists("%s/profile.bin" % path):
                try:
                    player_id = int(name)
                except ValueError:
                    continue
                break
        if not player_id:
            player_id = 1
        AnonUser.player_id = player_id
        login_manager.anonymous_user = AnonUser
    login_manager.init_app(app)

    @login_manager.user_loader
    def load_user(uid):
        return User.query.get(int(uid))

    send_message_thread = threading.Thread(target=send_server_back_online_message)
    send_message_thread.start()
    logger.info("Server is running.")
    server = WSGIServer(('0.0.0.0', 443), app, certfile='%s/cert-zwift-com.pem' % SSL_DIR, keyfile='%s/key-zwift-com.pem' % SSL_DIR, log=logger)
    server.serve_forever()

#    app.run(ssl_context=('%s/cert-zwift-com.pem' % SSL_DIR, '%s/key-zwift-com.pem' % SSL_DIR), port=443, threaded=True, host='0.0.0.0') # debug=True, use_reload=False)


if __name__ == "__main__":
    run_standalone({}, {}, None)
