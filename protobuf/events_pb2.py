# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: events.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


import profile_pb2 as profile__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x0c\x65vents.proto\x1a\rprofile.proto\"\x97\x01\n\x13InvitedProfileProto\x12\x0e\n\x06ipp_f1\x18\x01 \x02(\x04\x12\x0e\n\x06ipp_f2\x18\x02 \x01(\t\x12\x0e\n\x06ipp_f3\x18\x03 \x01(\t\x12\x0e\n\x06ipp_f4\x18\x04 \x01(\t\x12\x0e\n\x06ipp_f5\x18\x05 \x01(\x08\x12\x0e\n\x06ipp_f6\x18\x06 \x01(\x08\x12 \n\x0bplayer_type\x18\x07 \x01(\x0e\x32\x0b.PlayerType\"L\n\x10\x45ventInviteProto\x12%\n\x07profile\x18\x01 \x01(\x0b\x32\x14.InvitedProfileProto\x12\x11\n\tinvite_f2\x18\x02 \x02(\x04\"\xd8\x02\n\x15PrivateEventFeedProto\x12\x0e\n\x06pef_f1\x18\x01 \x02(\x04\x12\x15\n\x05sport\x18\x02 \x01(\x0e\x32\x06.Sport\x12\x1a\n\x12\x65ventSubgroupStart\x18\x03 \x01(\x04\x12\x10\n\x08route_id\x18\x04 \x01(\x04\x12\x0e\n\x06pef_f5\x18\x05 \x01(\r\x12\x0e\n\x06pef_f6\x18\x06 \x01(\x02\x12\x0e\n\x06pef_f7\x18\x07 \x01(\x04\x12\x0e\n\x06pef_f8\x18\x08 \x01(\r\x12\x0e\n\x06pef_f9\x18\t \x01(\r\x12\x0f\n\x07pef_f10\x18\n \x01(\r\x12\x0f\n\x07pef_f11\x18\x0b \x01(\t\x12\x0f\n\x07pef_f12\x18\x0c \x01(\x04\x12\x0f\n\x07pef_f13\x18\r \x01(\t\x12\x0f\n\x07pef_f14\x18\x0e \x01(\t\x12\x0f\n\x07pef_f15\x18\x0f \x01(\x04\x12\x12\n\nsubgroupId\x18\x10 \x01(\x04\x12\x0f\n\x07pef_f17\x18\x11 \x01(\r\x12\x0f\n\x07pef_f18\x18\x12 \x01(\x08\"@\n\x19PrivateEventFeedListProto\x12#\n\x03pef\x18\x01 \x03(\x0b\x32\x16.PrivateEventFeedProto\"H\n\x11PlayerJoinedEvent\x12\x10\n\x08\x65v_sg_id\x18\x01 \x02(\x04\x12\x11\n\tplayer_id\x18\x02 \x01(\x04\x12\x0e\n\x06pje_f3\x18\x03 \x01(\x04\"F\n\x0fPlayerLeftEvent\x12\x10\n\x08\x65v_sg_id\x18\x01 \x02(\x04\x12\x11\n\tplayer_id\x18\x02 \x01(\x04\x12\x0e\n\x06ple_f3\x18\x03 \x01(\x04\"\xd3\x07\n\x15\x45ventSubgroupProtobuf\x12\n\n\x02id\x18\x01 \x02(\x04\x12\x0c\n\x04name\x18\x02 \x01(\t\x12\x13\n\x0b\x64\x65scription\x18\x03 \x01(\t\x12\x0e\n\x06\x65vs_f4\x18\x04 \x01(\r\x12\x0e\n\x06\x65vs_f5\x18\x05 \x01(\r\x12\x0e\n\x06\x65vs_f6\x18\x06 \x01(\r\x12\r\n\x05scode\x18\x07 \x01(\t\x12\x10\n\x08rules_id\x18\x08 \x01(\x04\x12\x19\n\x11registrationStart\x18\t \x01(\x04\x12\x1b\n\x13registrationStartWT\x18\n \x01(\x04\x12\x17\n\x0fregistrationEnd\x18\x0b \x01(\x04\x12\x19\n\x11registrationEndWT\x18\x0c \x01(\x04\x12\x13\n\x0blineUpStart\x18\r \x01(\x04\x12\x15\n\rlineUpStartWT\x18\x0e \x01(\x04\x12\x11\n\tlineUpEnd\x18\x0f \x01(\x04\x12\x13\n\x0blineUpEndWT\x18\x10 \x01(\x04\x12\x1a\n\x12\x65ventSubgroupStart\x18\x11 \x01(\x04\x12\x1c\n\x14\x65ventSubgroupStartWT\x18\x12 \x01(\x04\x12\x0f\n\x07\x65vs_f19\x18\x13 \x01(\x04\x12\x0f\n\x07\x65vs_f20\x18\x14 \x01(\x04\x12\x0f\n\x07\x65vs_f21\x18\x15 \x01(\x08\x12\x10\n\x08route_id\x18\x16 \x02(\x04\x12\x16\n\x0einvitedLeaders\x18\x17 \x03(\x04\x12\x18\n\x10\x64istanceInMeters\x18\x18 \x01(\x02\x12\x0c\n\x04laps\x18\x19 \x01(\r\x12\x15\n\rstartLocation\x18\x1d \x01(\x04\x12\r\n\x05label\x18\x1e \x01(\r\x12\x10\n\x08paceType\x18\x1f \x01(\r\x12\x15\n\rfromPaceValue\x18  \x01(\x02\x12\x13\n\x0btoPaceValue\x18! \x01(\x02\x12\x19\n\x11\x64urationInSeconds\x18\" \x01(\r\x12\x0f\n\x07\x65vs_f35\x18# \x01(\r\x12\x12\n\njerseyHash\x18$ \x01(\x04\x12\x0f\n\x07\x65vs_f37\x18% \x01(\x08\x12\x0f\n\x07\x65vs_f38\x18& \x01(\r\x12\x14\n\x0c\x61uxiliaryUrl\x18\' \x01(\t\x12\x10\n\x08\x62ikeHash\x18( \x01(\x04\x12\x17\n\x0finvitedSweepers\x18) \x03(\x04\x12\x0f\n\x07\x65vs_f42\x18* \x01(\x04\x12\x11\n\tcustomUrl\x18+ \x01(\t\x12\x0f\n\x07\x65vs_f44\x18, \x01(\x08\x12\x0c\n\x04tags\x18- \x01(\t\x12\x19\n\x11lateJoinInMinutes\x18. \x01(\r\x12\x11\n\tcourse_id\x18/ \x01(\x04\x12\x0f\n\x07\x65vs_f48\x18\x30 \x01(\x04\x12\x10\n\x08routeUrl\x18\x31 \x01(\t\x12\x0f\n\x07\x65vs_f50\x18\x32 \x03(\x05\x12\x0f\n\x07\x65vs_f51\x18\x33 \x01(\x08\"g\n\x15MicroserviceEventData\x12\x0c\n\x04name\x18\x01 \x01(\t\x12\x1a\n\x12\x65xternalResourceId\x18\x02 \x01(\x0c\x12$\n\nvisibility\x18\x03 \x01(\x0e\x32\x10.EventVisibility\"D\n\x13\x45ventSeriesProtobuf\x12\n\n\x02id\x18\x01 \x01(\x04\x12\x0c\n\x04name\x18\x02 \x01(\t\x12\x13\n\x0b\x64\x65scription\x18\x03 \x01(\t\"\x7f\n\x15\x45ventTimeTrialOptions\x12\x1c\n\x14timeGapBetweenRowsMs\x18\x01 \x01(\r\x12\x0f\n\x07maxRows\x18\x02 \x01(\r\x12\x17\n\x0fmaxRidersPerRow\x18\x03 \x01(\r\x12\x0e\n\x06\x65vt_f4\x18\x04 \x01(\r\x12\x0e\n\x06\x65vt_f5\x18\x05 \x01(\x04\"\xcf\x06\n\x05\x45vent\x12\n\n\x02id\x18\x01 \x02(\x04\x12\x14\n\x0cserver_realm\x18\x02 \x01(\x04\x12\x0c\n\x04name\x18\x03 \x02(\t\x12\x13\n\x0b\x64\x65scription\x18\x04 \x01(\t\x12\x12\n\neventStart\x18\x05 \x01(\x04\x12\x0c\n\x04\x65_f6\x18\x06 \x01(\t\x12\x18\n\x10\x64istanceInMeters\x18\x07 \x01(\x02\x12\x0c\n\x04laps\x18\x08 \x01(\r\x12\x0c\n\x04\x65_f9\x18\t \x01(\r\x12(\n\x08\x63\x61tegory\x18\n \x03(\x0b\x32\x16.EventSubgroupProtobuf\x12\r\n\x05\x65_f11\x18\x0b \x01(\t\x12\x10\n\x08imageUrl\x18\x0c \x01(\t\x12\x19\n\x11\x64urationInSeconds\x18\r \x01(\r\x12\x10\n\x08route_id\x18\x0e \x01(\x04\x12\x10\n\x08rules_id\x18\x0f \x01(\x04\x12\r\n\x05\x65_f16\x18\x10 \x01(\r\x12\x0f\n\x07visible\x18\x11 \x01(\x08\x12\x12\n\njerseyHash\x18\x12 \x01(\x04\x12\r\n\x05\x65_f19\x18\x13 \x01(\t\x12\r\n\x05\x65_f20\x18\x14 \x01(\t\x12\r\n\x05\x65_f21\x18\x15 \x01(\t\x12\x15\n\x05sport\x18\x16 \x01(\x0e\x32\x06.Sport\x12\r\n\x05\x65_f23\x18\x17 \x01(\x04\x12\x1d\n\teventType\x18\x18 \x01(\x0e\x32\n.EventType\x12\r\n\x05\x65_f25\x18\x19 \x01(\x04\x12\r\n\x05\x65_f26\x18\x1a \x01(\t\x12\r\n\x05\x65_f27\x18\x1b \x01(\x04\x12\x1e\n\x16overrideMapPreferences\x18\x1c \x01(\x08\x12\"\n\x1ainvisibleToNonParticipants\x18\x1d \x01(\x08\x12&\n\x08\x65vSeries\x18\x1e \x01(\x0b\x32\x14.EventSeriesProtobuf\x12\x0c\n\x04tags\x18\x1f \x01(\t\x12\r\n\x05\x65_f32\x18  \x01(\x04\x12\x0e\n\x06\x65_wtrl\x18! \x01(\x08\x12\x19\n\x11lateJoinInMinutes\x18\" \x01(\r\x12\x11\n\tcourse_id\x18# \x01(\x04\x12#\n\x03tto\x18$ \x01(\x0b\x32\x16.EventTimeTrialOptions\x12\r\n\x05\x65_f37\x18% \x01(\t\x12\r\n\x05\x65_f38\x18& \x01(\t\x12\r\n\x05\x65_f39\x18\' \x01(\r\x12$\n\x04msed\x18( \x01(\x0b\x32\x16.MicroserviceEventData\x12\r\n\x05\x65_f41\x18) \x03(\r\" \n\x06\x45vents\x12\x16\n\x06\x65vents\x18\x01 \x03(\x0b\x32\x06.Event*V\n\x0f\x45ventVisibility\x12\x0b\n\x07\x45V_NULL\x10\x00\x12\x10\n\x0c\x45V_PUB_SHARE\x10\x01\x12\x12\n\x0e\x45V_BY_RESOURCE\x10\x02\x12\x10\n\x0c\x45V_SHAREABLE\x10\x03*\xa5\x01\n\x0b\x45ventTypeV2\x12\x16\n\x12\x45VENT_TYPE_UNKNOWN\x10\x00\x12\x15\n\x11\x45VENT_TYPE_EFONDO\x10\x01\x12\x13\n\x0f\x45VENT_TYPE_RACE\x10\x02\x12\x19\n\x15\x45VENT_TYPE_GROUP_RIDE\x10\x03\x12\x1c\n\x18\x45VENT_TYPE_GROUP_WORKOUT\x10\x04\x12\x19\n\x15\x45VENT_TYPE_TIME_TRIAL\x10\x05*d\n\tEventType\x12\x0e\n\nET_UNKNOWN\x10\x00\x12\n\n\x06\x45\x46ONDO\x10\x01\x12\x08\n\x04RACE\x10\x02\x12\x0e\n\nGROUP_RIDE\x10\x03\x12\x11\n\rGROUP_WORKOUT\x10\x04\x12\x0e\n\nTIME_TRIAL\x10\x05*o\n\x0c\x45ventCulling\x12\x15\n\x11\x43ULLING_UNDEFINED\x10\x00\x12\x15\n\x11\x43ULLING_EVERYBODY\x10\x01\x12\x16\n\x12\x43ULLING_EVENT_ONLY\x10\x02\x12\x19\n\x15\x43ULLING_SUBGROUP_ONLY\x10\x03')

_EVENTVISIBILITY = DESCRIPTOR.enum_types_by_name['EventVisibility']
EventVisibility = enum_type_wrapper.EnumTypeWrapper(_EVENTVISIBILITY)
_EVENTTYPEV2 = DESCRIPTOR.enum_types_by_name['EventTypeV2']
EventTypeV2 = enum_type_wrapper.EnumTypeWrapper(_EVENTTYPEV2)
_EVENTTYPE = DESCRIPTOR.enum_types_by_name['EventType']
EventType = enum_type_wrapper.EnumTypeWrapper(_EVENTTYPE)
_EVENTCULLING = DESCRIPTOR.enum_types_by_name['EventCulling']
EventCulling = enum_type_wrapper.EnumTypeWrapper(_EVENTCULLING)
EV_NULL = 0
EV_PUB_SHARE = 1
EV_BY_RESOURCE = 2
EV_SHAREABLE = 3
EVENT_TYPE_UNKNOWN = 0
EVENT_TYPE_EFONDO = 1
EVENT_TYPE_RACE = 2
EVENT_TYPE_GROUP_RIDE = 3
EVENT_TYPE_GROUP_WORKOUT = 4
EVENT_TYPE_TIME_TRIAL = 5
ET_UNKNOWN = 0
EFONDO = 1
RACE = 2
GROUP_RIDE = 3
GROUP_WORKOUT = 4
TIME_TRIAL = 5
CULLING_UNDEFINED = 0
CULLING_EVERYBODY = 1
CULLING_EVENT_ONLY = 2
CULLING_SUBGROUP_ONLY = 3


_INVITEDPROFILEPROTO = DESCRIPTOR.message_types_by_name['InvitedProfileProto']
_EVENTINVITEPROTO = DESCRIPTOR.message_types_by_name['EventInviteProto']
_PRIVATEEVENTFEEDPROTO = DESCRIPTOR.message_types_by_name['PrivateEventFeedProto']
_PRIVATEEVENTFEEDLISTPROTO = DESCRIPTOR.message_types_by_name['PrivateEventFeedListProto']
_PLAYERJOINEDEVENT = DESCRIPTOR.message_types_by_name['PlayerJoinedEvent']
_PLAYERLEFTEVENT = DESCRIPTOR.message_types_by_name['PlayerLeftEvent']
_EVENTSUBGROUPPROTOBUF = DESCRIPTOR.message_types_by_name['EventSubgroupProtobuf']
_MICROSERVICEEVENTDATA = DESCRIPTOR.message_types_by_name['MicroserviceEventData']
_EVENTSERIESPROTOBUF = DESCRIPTOR.message_types_by_name['EventSeriesProtobuf']
_EVENTTIMETRIALOPTIONS = DESCRIPTOR.message_types_by_name['EventTimeTrialOptions']
_EVENT = DESCRIPTOR.message_types_by_name['Event']
_EVENTS = DESCRIPTOR.message_types_by_name['Events']
InvitedProfileProto = _reflection.GeneratedProtocolMessageType('InvitedProfileProto', (_message.Message,), {
  'DESCRIPTOR' : _INVITEDPROFILEPROTO,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:InvitedProfileProto)
  })
_sym_db.RegisterMessage(InvitedProfileProto)

EventInviteProto = _reflection.GeneratedProtocolMessageType('EventInviteProto', (_message.Message,), {
  'DESCRIPTOR' : _EVENTINVITEPROTO,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:EventInviteProto)
  })
_sym_db.RegisterMessage(EventInviteProto)

PrivateEventFeedProto = _reflection.GeneratedProtocolMessageType('PrivateEventFeedProto', (_message.Message,), {
  'DESCRIPTOR' : _PRIVATEEVENTFEEDPROTO,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:PrivateEventFeedProto)
  })
_sym_db.RegisterMessage(PrivateEventFeedProto)

PrivateEventFeedListProto = _reflection.GeneratedProtocolMessageType('PrivateEventFeedListProto', (_message.Message,), {
  'DESCRIPTOR' : _PRIVATEEVENTFEEDLISTPROTO,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:PrivateEventFeedListProto)
  })
_sym_db.RegisterMessage(PrivateEventFeedListProto)

PlayerJoinedEvent = _reflection.GeneratedProtocolMessageType('PlayerJoinedEvent', (_message.Message,), {
  'DESCRIPTOR' : _PLAYERJOINEDEVENT,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:PlayerJoinedEvent)
  })
_sym_db.RegisterMessage(PlayerJoinedEvent)

PlayerLeftEvent = _reflection.GeneratedProtocolMessageType('PlayerLeftEvent', (_message.Message,), {
  'DESCRIPTOR' : _PLAYERLEFTEVENT,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:PlayerLeftEvent)
  })
_sym_db.RegisterMessage(PlayerLeftEvent)

EventSubgroupProtobuf = _reflection.GeneratedProtocolMessageType('EventSubgroupProtobuf', (_message.Message,), {
  'DESCRIPTOR' : _EVENTSUBGROUPPROTOBUF,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:EventSubgroupProtobuf)
  })
_sym_db.RegisterMessage(EventSubgroupProtobuf)

MicroserviceEventData = _reflection.GeneratedProtocolMessageType('MicroserviceEventData', (_message.Message,), {
  'DESCRIPTOR' : _MICROSERVICEEVENTDATA,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:MicroserviceEventData)
  })
_sym_db.RegisterMessage(MicroserviceEventData)

EventSeriesProtobuf = _reflection.GeneratedProtocolMessageType('EventSeriesProtobuf', (_message.Message,), {
  'DESCRIPTOR' : _EVENTSERIESPROTOBUF,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:EventSeriesProtobuf)
  })
_sym_db.RegisterMessage(EventSeriesProtobuf)

EventTimeTrialOptions = _reflection.GeneratedProtocolMessageType('EventTimeTrialOptions', (_message.Message,), {
  'DESCRIPTOR' : _EVENTTIMETRIALOPTIONS,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:EventTimeTrialOptions)
  })
_sym_db.RegisterMessage(EventTimeTrialOptions)

Event = _reflection.GeneratedProtocolMessageType('Event', (_message.Message,), {
  'DESCRIPTOR' : _EVENT,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:Event)
  })
_sym_db.RegisterMessage(Event)

Events = _reflection.GeneratedProtocolMessageType('Events', (_message.Message,), {
  'DESCRIPTOR' : _EVENTS,
  '__module__' : 'events_pb2'
  # @@protoc_insertion_point(class_scope:Events)
  })
_sym_db.RegisterMessage(Events)

if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _EVENTVISIBILITY._serialized_start=2992
  _EVENTVISIBILITY._serialized_end=3078
  _EVENTTYPEV2._serialized_start=3081
  _EVENTTYPEV2._serialized_end=3246
  _EVENTTYPE._serialized_start=3248
  _EVENTTYPE._serialized_end=3348
  _EVENTCULLING._serialized_start=3350
  _EVENTCULLING._serialized_end=3461
  _INVITEDPROFILEPROTO._serialized_start=32
  _INVITEDPROFILEPROTO._serialized_end=183
  _EVENTINVITEPROTO._serialized_start=185
  _EVENTINVITEPROTO._serialized_end=261
  _PRIVATEEVENTFEEDPROTO._serialized_start=264
  _PRIVATEEVENTFEEDPROTO._serialized_end=608
  _PRIVATEEVENTFEEDLISTPROTO._serialized_start=610
  _PRIVATEEVENTFEEDLISTPROTO._serialized_end=674
  _PLAYERJOINEDEVENT._serialized_start=676
  _PLAYERJOINEDEVENT._serialized_end=748
  _PLAYERLEFTEVENT._serialized_start=750
  _PLAYERLEFTEVENT._serialized_end=820
  _EVENTSUBGROUPPROTOBUF._serialized_start=823
  _EVENTSUBGROUPPROTOBUF._serialized_end=1802
  _MICROSERVICEEVENTDATA._serialized_start=1804
  _MICROSERVICEEVENTDATA._serialized_end=1907
  _EVENTSERIESPROTOBUF._serialized_start=1909
  _EVENTSERIESPROTOBUF._serialized_end=1977
  _EVENTTIMETRIALOPTIONS._serialized_start=1979
  _EVENTTIMETRIALOPTIONS._serialized_end=2106
  _EVENT._serialized_start=2109
  _EVENT._serialized_end=2956
  _EVENTS._serialized_start=2958
  _EVENTS._serialized_end=2990
# @@protoc_insertion_point(module_scope)
