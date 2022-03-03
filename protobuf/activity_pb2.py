# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: activity.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x0e\x61\x63tivity.proto\"\x9b\x01\n\rNotableMoment\x12\x13\n\x0b\x61\x63tivity_id\x18\x01 \x01(\x04\x12\"\n\x04type\x18\x02 \x01(\x0e\x32\x14.NotableMomentTypeZG\x12\x10\n\x08priority\x18\x03 \x01(\r\x12\x0c\n\x04time\x18\x04 \x01(\x04\x12\x0c\n\x04\x61ux1\x18\x05 \x01(\t\x12\x0c\n\x04\x61ux2\x18\x06 \x01(\t\x12\x15\n\rlargeImageUrl\x18\x07 \x01(\t\"U\n\x11SocialInteraction\x12\x11\n\tplayer_id\x18\x01 \x01(\x04\x12\x0f\n\x07seconds\x18\x02 \x01(\r\x12\r\n\x05score\x18\x03 \x01(\x02\x12\r\n\x05si_f4\x18\x04 \x01(\t\".\n\x0f\x43lubAttribution\x12\x0c\n\x04name\x18\x01 \x01(\t\x12\r\n\x05value\x18\x02 \x01(\x02\"\xe5\x03\n\x08\x41\x63tivity\x12\n\n\x02id\x18\x01 \x01(\x04\x12\x11\n\tplayer_id\x18\x02 \x02(\x04\x12\n\n\x02\x66\x33\x18\x03 \x02(\x04\x12\x0c\n\x04name\x18\x04 \x02(\t\x12\n\n\x02\x66\x35\x18\x05 \x01(\t\x12\n\n\x02\x66\x36\x18\x06 \x01(\x08\x12\x12\n\nstart_date\x18\x07 \x02(\t\x12\x10\n\x08\x65nd_date\x18\x08 \x01(\t\x12\x10\n\x08\x64istance\x18\t \x01(\x02\x12\x16\n\x0e\x61vg_heart_rate\x18\n \x01(\x02\x12\x16\n\x0emax_heart_rate\x18\x0b \x01(\x02\x12\x11\n\tavg_watts\x18\x0c \x01(\x02\x12\x11\n\tmax_watts\x18\r \x01(\x02\x12\x13\n\x0b\x61vg_cadence\x18\x0e \x01(\x02\x12\x13\n\x0bmax_cadence\x18\x0f \x01(\x02\x12\x11\n\tavg_speed\x18\x10 \x01(\x02\x12\x11\n\tmax_speed\x18\x11 \x01(\x02\x12\x10\n\x08\x63\x61lories\x18\x12 \x01(\x02\x12\x17\n\x0ftotal_elevation\x18\x13 \x01(\x02\x12\x18\n\x10strava_upload_id\x18\x14 \x01(\r\x12\x1a\n\x12strava_activity_id\x18\x15 \x01(\r\x12\x0b\n\x03\x66\x32\x33\x18\x17 \x01(\r\x12\x0b\n\x03\x66it\x18\x18 \x01(\x0c\x12\x14\n\x0c\x66it_filename\x18\x19 \x01(\t\x12\x0b\n\x03\x66\x32\x39\x18\x1d \x01(\x03\x12\x0c\n\x04\x64\x61te\x18\x1f \x01(\t\"-\n\x0c\x41\x63tivityList\x12\x1d\n\nactivities\x18\x01 \x03(\x0b\x32\t.Activity*\xe5\x02\n\x14NotableMomentTypeZCA\x12\x1d\n\x19NMTC_ACHIEVEMENT_UNLOCKED\x10\x01\x12\x16\n\x12NMTC_UNLOCKED_ITEM\x10\x02\x12\x1a\n\x16NMTC_MISSION_COMPLETED\x10\x03\x12\x1b\n\x17NMTC_FINISHED_CHALLENGE\x10\x04\x12\x19\n\x15NMTC_TOOK_ARCH_JERSEY\x10\x05\x12\x0f\n\x0bNMTC_NEW_PR\x10\x06\x12\x19\n\x15NMTC_MET_DAILY_TARGET\x10\x07\x12\x15\n\x11NMTC_GAINED_LEVEL\x10\x08\x12\x17\n\x13NMTC_COMPLETED_GOAL\x10\t\x12\x17\n\x13NMTC_FINISHED_EVENT\x10\n\x12\x19\n\x15NMTC_FINISHED_WORKOUT\x10\x0b\x12\x10\n\x0cNMTC_RIDE_ON\x10\x0c\x12 \n\x1cNMTC_TRAINING_PLAN_COMPLETED\x10\r*\xf2\x04\n\x17NotableMomentTypeZG_idx\x12\x10\n\x0cNMTI_UNKNOWN\x10\x00\x12\x0f\n\x0bNMTI_NEW_PR\x10\x01\x12\x15\n\x11NMTI_GAINED_LEVEL\x10\x02\x12\x1f\n\x1bNMTI_TRAINING_PLAN_COMPLETE\x10\x03\x12\x16\n\x12NMTI_UNLOCKED_ITEM\x10\x04\x12\x1d\n\x19NMTI_ACHIEVEMENT_UNLOCKED\x10\x05\x12\x1a\n\x16NMTI_MISSION_COMPLETED\x10\x06\x12\x17\n\x13NMTI_COMPLETED_GOAL\x10\x07\x12\x19\n\x15NMTI_MET_DAILY_TARGET\x10\x08\x12\x19\n\x15NMTI_TOOK_ARCH_JERSEY\x10\t\x12\x1b\n\x17NMTI_FINISHED_CHALLENGE\x10\n\x12\x17\n\x13NMTI_FINISHED_EVENT\x10\x0b\x12\x19\n\x15NMTI_FINISHED_WORKOUT\x10\x0c\x12\x17\n\x13NMTI_ACTIVITY_BESTS\x10\r\x12\x0f\n\x0bNMTI_RIDEON\x10\x0e\x12\x13\n\x0fNMTI_RIDEON_INT\x10\x0f\x12\x13\n\x0fNMTI_QUIT_EVENT\x10\x10\x12\x15\n\x11NMTI_USED_POWERUP\x10\x11\x12\x1b\n\x17NMTI_PASSED_TIMING_ARCH\x10\x12\x12\x15\n\x11NMTI_CREATED_GOAL\x10\x13\x12\x15\n\x11NMTI_JOINED_EVENT\x10\x14\x12\x18\n\x14NMTI_STARTED_WORKOUT\x10\x15\x12\x18\n\x14NMTI_STARTED_MISSION\x10\x16\x12\x1f\n\x1bNMTI_HOLIDAY_EVENT_COMPLETE\x10\x17*\xc5\x04\n\x13NotableMomentTypeZG\x12\x0e\n\nNMT_NEW_PR\x10\x00\x12\x14\n\x10NMT_GAINED_LEVEL\x10\x05\x12\x1e\n\x1aNMT_TRAINING_PLAN_COMPLETE\x10\x13\x12\x15\n\x11NMT_UNLOCKED_ITEM\x10\x04\x12\x1c\n\x18NMT_ACHIEVEMENT_UNLOCKED\x10\x02\x12\x19\n\x15NMT_MISSION_COMPLETED\x10\x03\x12\x16\n\x12NMT_COMPLETED_GOAL\x10\n\x12\x18\n\x14NMT_MET_DAILY_TARGET\x10\x01\x12\x18\n\x14NMT_TOOK_ARCH_JERSEY\x10\x08\x12\x1a\n\x16NMT_FINISHED_CHALLENGE\x10\x11\x12\x16\n\x12NMT_FINISHED_EVENT\x10\r\x12\x18\n\x14NMT_FINISHED_WORKOUT\x10\x0f\x12\x16\n\x12NMT_ACTIVITY_BESTS\x10\x14\x12\x0e\n\nNMT_RIDEON\x10\x12\x12\x12\n\x0eNMT_RIDEON_INT\x10\x16\x12\x12\n\x0eNMT_QUIT_EVENT\x10\x0c\x12\x14\n\x10NMT_USED_POWERUP\x10\x06\x12\x1a\n\x16NMT_PASSED_TIMING_ARCH\x10\x07\x12\x14\n\x10NMT_CREATED_GOAL\x10\t\x12\x14\n\x10NMT_JOINED_EVENT\x10\x0b\x12\x17\n\x13NMT_STARTED_WORKOUT\x10\x0e\x12\x17\n\x13NMT_STARTED_MISSION\x10\x10\x12\x1e\n\x1aNMT_HOLIDAY_EVENT_COMPLETE\x10\x15*\xae\x01\n\x13ProfileFollowStatus\x12\x0f\n\x0bPFS_UNKNOWN\x10\x01\x12\x1a\n\x16PFS_REQUESTS_TO_FOLLOW\x10\x02\x12\x14\n\x10PFS_IS_FOLLOWING\x10\x03\x12\x12\n\x0ePFS_IS_BLOCKED\x10\x04\x12\x17\n\x13PFS_NO_RELATIONSHIP\x10\x05\x12\x0c\n\x08PFS_SELF\x10\x06\x12\x19\n\x15PFS_HAS_BEEN_DECLINED\x10\x07*J\n\x0e\x46itnessPrivacy\x12\t\n\x05UNSET\x10\x00\x12\x17\n\x13HIDE_SENSITIVE_DATA\x10\x01\x12\x14\n\x10SAME_AS_ACTIVITY\x10\x02')

_NOTABLEMOMENTTYPEZCA = DESCRIPTOR.enum_types_by_name['NotableMomentTypeZCA']
NotableMomentTypeZCA = enum_type_wrapper.EnumTypeWrapper(_NOTABLEMOMENTTYPEZCA)
_NOTABLEMOMENTTYPEZG_IDX = DESCRIPTOR.enum_types_by_name['NotableMomentTypeZG_idx']
NotableMomentTypeZG_idx = enum_type_wrapper.EnumTypeWrapper(_NOTABLEMOMENTTYPEZG_IDX)
_NOTABLEMOMENTTYPEZG = DESCRIPTOR.enum_types_by_name['NotableMomentTypeZG']
NotableMomentTypeZG = enum_type_wrapper.EnumTypeWrapper(_NOTABLEMOMENTTYPEZG)
_PROFILEFOLLOWSTATUS = DESCRIPTOR.enum_types_by_name['ProfileFollowStatus']
ProfileFollowStatus = enum_type_wrapper.EnumTypeWrapper(_PROFILEFOLLOWSTATUS)
_FITNESSPRIVACY = DESCRIPTOR.enum_types_by_name['FitnessPrivacy']
FitnessPrivacy = enum_type_wrapper.EnumTypeWrapper(_FITNESSPRIVACY)
NMTC_ACHIEVEMENT_UNLOCKED = 1
NMTC_UNLOCKED_ITEM = 2
NMTC_MISSION_COMPLETED = 3
NMTC_FINISHED_CHALLENGE = 4
NMTC_TOOK_ARCH_JERSEY = 5
NMTC_NEW_PR = 6
NMTC_MET_DAILY_TARGET = 7
NMTC_GAINED_LEVEL = 8
NMTC_COMPLETED_GOAL = 9
NMTC_FINISHED_EVENT = 10
NMTC_FINISHED_WORKOUT = 11
NMTC_RIDE_ON = 12
NMTC_TRAINING_PLAN_COMPLETED = 13
NMTI_UNKNOWN = 0
NMTI_NEW_PR = 1
NMTI_GAINED_LEVEL = 2
NMTI_TRAINING_PLAN_COMPLETE = 3
NMTI_UNLOCKED_ITEM = 4
NMTI_ACHIEVEMENT_UNLOCKED = 5
NMTI_MISSION_COMPLETED = 6
NMTI_COMPLETED_GOAL = 7
NMTI_MET_DAILY_TARGET = 8
NMTI_TOOK_ARCH_JERSEY = 9
NMTI_FINISHED_CHALLENGE = 10
NMTI_FINISHED_EVENT = 11
NMTI_FINISHED_WORKOUT = 12
NMTI_ACTIVITY_BESTS = 13
NMTI_RIDEON = 14
NMTI_RIDEON_INT = 15
NMTI_QUIT_EVENT = 16
NMTI_USED_POWERUP = 17
NMTI_PASSED_TIMING_ARCH = 18
NMTI_CREATED_GOAL = 19
NMTI_JOINED_EVENT = 20
NMTI_STARTED_WORKOUT = 21
NMTI_STARTED_MISSION = 22
NMTI_HOLIDAY_EVENT_COMPLETE = 23
NMT_NEW_PR = 0
NMT_GAINED_LEVEL = 5
NMT_TRAINING_PLAN_COMPLETE = 19
NMT_UNLOCKED_ITEM = 4
NMT_ACHIEVEMENT_UNLOCKED = 2
NMT_MISSION_COMPLETED = 3
NMT_COMPLETED_GOAL = 10
NMT_MET_DAILY_TARGET = 1
NMT_TOOK_ARCH_JERSEY = 8
NMT_FINISHED_CHALLENGE = 17
NMT_FINISHED_EVENT = 13
NMT_FINISHED_WORKOUT = 15
NMT_ACTIVITY_BESTS = 20
NMT_RIDEON = 18
NMT_RIDEON_INT = 22
NMT_QUIT_EVENT = 12
NMT_USED_POWERUP = 6
NMT_PASSED_TIMING_ARCH = 7
NMT_CREATED_GOAL = 9
NMT_JOINED_EVENT = 11
NMT_STARTED_WORKOUT = 14
NMT_STARTED_MISSION = 16
NMT_HOLIDAY_EVENT_COMPLETE = 21
PFS_UNKNOWN = 1
PFS_REQUESTS_TO_FOLLOW = 2
PFS_IS_FOLLOWING = 3
PFS_IS_BLOCKED = 4
PFS_NO_RELATIONSHIP = 5
PFS_SELF = 6
PFS_HAS_BEEN_DECLINED = 7
UNSET = 0
HIDE_SENSITIVE_DATA = 1
SAME_AS_ACTIVITY = 2


_NOTABLEMOMENT = DESCRIPTOR.message_types_by_name['NotableMoment']
_SOCIALINTERACTION = DESCRIPTOR.message_types_by_name['SocialInteraction']
_CLUBATTRIBUTION = DESCRIPTOR.message_types_by_name['ClubAttribution']
_ACTIVITY = DESCRIPTOR.message_types_by_name['Activity']
_ACTIVITYLIST = DESCRIPTOR.message_types_by_name['ActivityList']
NotableMoment = _reflection.GeneratedProtocolMessageType('NotableMoment', (_message.Message,), {
  'DESCRIPTOR' : _NOTABLEMOMENT,
  '__module__' : 'activity_pb2'
  # @@protoc_insertion_point(class_scope:NotableMoment)
  })
_sym_db.RegisterMessage(NotableMoment)

SocialInteraction = _reflection.GeneratedProtocolMessageType('SocialInteraction', (_message.Message,), {
  'DESCRIPTOR' : _SOCIALINTERACTION,
  '__module__' : 'activity_pb2'
  # @@protoc_insertion_point(class_scope:SocialInteraction)
  })
_sym_db.RegisterMessage(SocialInteraction)

ClubAttribution = _reflection.GeneratedProtocolMessageType('ClubAttribution', (_message.Message,), {
  'DESCRIPTOR' : _CLUBATTRIBUTION,
  '__module__' : 'activity_pb2'
  # @@protoc_insertion_point(class_scope:ClubAttribution)
  })
_sym_db.RegisterMessage(ClubAttribution)

Activity = _reflection.GeneratedProtocolMessageType('Activity', (_message.Message,), {
  'DESCRIPTOR' : _ACTIVITY,
  '__module__' : 'activity_pb2'
  # @@protoc_insertion_point(class_scope:Activity)
  })
_sym_db.RegisterMessage(Activity)

ActivityList = _reflection.GeneratedProtocolMessageType('ActivityList', (_message.Message,), {
  'DESCRIPTOR' : _ACTIVITYLIST,
  '__module__' : 'activity_pb2'
  # @@protoc_insertion_point(class_scope:ActivityList)
  })
_sym_db.RegisterMessage(ActivityList)

if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _NOTABLEMOMENTTYPEZCA._serialized_start=847
  _NOTABLEMOMENTTYPEZCA._serialized_end=1204
  _NOTABLEMOMENTTYPEZG_IDX._serialized_start=1207
  _NOTABLEMOMENTTYPEZG_IDX._serialized_end=1833
  _NOTABLEMOMENTTYPEZG._serialized_start=1836
  _NOTABLEMOMENTTYPEZG._serialized_end=2417
  _PROFILEFOLLOWSTATUS._serialized_start=2420
  _PROFILEFOLLOWSTATUS._serialized_end=2594
  _FITNESSPRIVACY._serialized_start=2596
  _FITNESSPRIVACY._serialized_end=2670
  _NOTABLEMOMENT._serialized_start=19
  _NOTABLEMOMENT._serialized_end=174
  _SOCIALINTERACTION._serialized_start=176
  _SOCIALINTERACTION._serialized_end=261
  _CLUBATTRIBUTION._serialized_start=263
  _CLUBATTRIBUTION._serialized_end=309
  _ACTIVITY._serialized_start=312
  _ACTIVITY._serialized_end=797
  _ACTIVITYLIST._serialized_start=799
  _ACTIVITYLIST._serialized_end=844
# @@protoc_insertion_point(module_scope)
