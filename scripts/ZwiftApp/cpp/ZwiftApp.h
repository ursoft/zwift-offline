#pragma once
#include "pch.h"

#include "FeatureRequest.pb.h"
#include "Achievements.pb.h"
#include "Profile.pb.h"
#include "Goal.pb.h"
#include "PerSessionInfo.pb.h"
#include "Login.pb.h"
#include "SegmentResults.pb.h"
#include "UdpConfig.pb.h"
#include "PlayerState.pb.h"
#include "TcpConfig.pb.h"
#include "Event.pb.h"
#include "ServerToClient.pb.h"
#include "PlayerSummary.pb.h"
#include "WorldAttribute.pb.h"
#include "HashSeed.pb.h"
#include "World.pb.h"
#include "Activity.pb.h"
#include "ClientToServer.pb.h"
#include "PhoneAux.pb.h"
#include "Playback.pb.h"
#include "RouteResult.pb.h"
#include "Social.pb.h"
#include "Campaign.pb.h"
#include "Club.pb.h"
#include "Mapping.pb.h"
#include "RaceResult.pb.h"
#include "UUID.pb.h"
#include "Workout.pb.h"
#include "Zfiles.pb.h"
#include "Aggregation.pb.h"
using protobuf_bytes = std::string; //it is better for Google to make it a vector, but...
using NOTABLEMOMENT_TYPE = protobuf::NotableMomentType; //not sure
#include "MAT.h"
#include "DateTime.h"
#include "Logging.h"
#include "GameCritical.h"
#include "ZMutex.h"
#include "Entity.h"
#include "ZU.h"
#include "ZNoesis.h"
#include "Console.h"
#include "RenderTarget.h"
#include "Audio.h"
#include "GameWorld.h"
#include "EventSystem.h"
#include "CrashReporting.h"
#include "Experimentation.h"
#include "FitnessDeviceManager.h"
#include "BLEModule.h"
#include "Thread.h"
#include "XMLDoc.h"
#include "ZStringUtil.h"
#include "CRC.h"
#include "ZNet.h"
#include "PlayerAchievementService.h"
#include "NoesisPerfAnalytics.h"
#include "ClientTelemetry.h"
#include "GroupEvents.h"
#include "VideoCapture.h"
#include "Powerups.h"
#include "IoCPP.h"
#include "GoalsManager.h"
#include "UnitTypeManager.h"
#include "SaveActivityService.h"
#include "ConnectionManager.h"
#include "Localization.h"
#include "HoloReplayManager.h"
#include "DataRecorder.h"
#include "AccessoryManager.h"
#include "StaticAssetLoader.h"
#include "BikeManager.h"
#include "PlayerProfileCache.h"
#include "Road.h"
#include "EbikeBoost.h"
#include "BikeEntity.h"
#include "GFX.h"
#include "Downloader.h"
#include "OS.h"
#include "GAME.h"
//OMIT #include "tHigFile.h"
#include "ICU.h"
#include "CameraManager.h"
#include "NotableMomentsManager.h"
#include "WADManager.h"
#include "ProfanityFilter.h"
#include "RaceDictionary.h"
#include "ANIM.h"
#include "JM.h"
#include "LanExerciseDeviceManager.h"
#include "VRAM.h"
#include "GDE.h"
#include "MATERIAL.h"
#include "CFont2D.h"
#include "GameHolidayManager.h"
#include "ANTRECEIVER.h"
#include "GUI.h"
#include "UI_Dialogs.h"
#include "GNScene.h"
#include "LOADER.h"
#include "GNView.h"
#include "PostFX.h"
#include "Sky.h"
#include "Weather.h"
#include "ParticulateManager.h"
#include "SteeringModule.h"
#include "HUD.h"
#include "INSTANCING.h"
#include "Bib.h"
#include "ZwiftPowers.h"
#include "Databases.h"
#include "Stopwatch.h"
#include "PlayerStateHelper.h"
#include "Bots.h"
#include "SaveGame.h"
#include "ZML.h"
#include "Leaderboards.h"
#include "HeightMapManager.h"
#include "Joystick.h"
#include "RoadMover.h"
#include "Prefab.h"
#include "Video.h"
#include "TrainingPlan.h"
#include "Workout.h"
#include "ScriptedSessionManager.h"