-- Pathing for Alshirr Banebreath Entry: 14340
SET @NPC=14340;
SET @GUID=51894;
SET @PATH=@GUID * 10;
UPDATE `creature` SET `position_x`=4026.523,`position_y`=-555.293,`position_z`=342.505,`wander_distance`=0,`MovementType`=2 WHERE `guid`=@GUID;
UPDATE `creature_template` SET `MovementType`=2 WHERE `entry`=@NPC;
DELETE FROM `creature_addon` WHERE `guid`=@GUID;
INSERT INTO `creature_addon` (`guid`, `path_id`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(@GUID, @PATH, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL);
DELETE FROM `waypoint_data` WHERE `id`=@PATH;
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES
(@PATH,1,4026.523,-555.293,342.505,NULL,0,0,0,100,0),
(@PATH,2,3999.6077,-585.43585,338.0969,NULL,0,0,0,100,0),
(@PATH,3,3967.8643,-599.3677,338.82224,NULL,0,0,0,100,0),
(@PATH,4,3939.5344,-610.4964,340.5994,NULL,0,0,0,100,0),
(@PATH,5,3918.8508,-620.3714,339.56857,NULL,0,0,0,100,0),
(@PATH,6,3893.1277,-649.1434,334.0132,NULL,0,0,0,100,0),
(@PATH,7,3867.4185,-662.55194,330.74258,NULL,0,0,0,100,0),
(@PATH,8,3851.004,-677.2051,328.14557,NULL,0,0,0,100,0),
(@PATH,9,3841.1611,-713.5064,325.38644,NULL,0,0,0,100,0),
(@PATH,10,3836.204,-740.03125,319.4985,NULL,0,0,0,100,0),
(@PATH,11,3827.0447,-763.3758,314.87888,NULL,0,0,0,100,0),
(@PATH,12,3818.8672,-787.662,307.59967,NULL,0,0,0,100,0),
(@PATH,13,3792.5173,-812.1507,307.89844,NULL,0,0,0,100,0),
(@PATH,14,3768.1362,-831.72095,310.0974,NULL,0,0,0,100,0),
(@PATH,15,3771.9072,-848.68176,305.3522,NULL,0,0,0,100,0),
(@PATH,16,3757.2969,-866.6099,313.99258,NULL,0,0,0,100,0),
(@PATH,17,3771.9072,-848.68176,305.3522,NULL,0,0,0,100,0),
(@PATH,18,3768.1362,-831.72095,310.0974,NULL,0,0,0,100,0),
(@PATH,19,3792.5173,-812.1507,307.89844,NULL,0,0,0,100,0),
(@PATH,20,3818.8672,-787.662,307.59967,NULL,0,0,0,100,0),
(@PATH,21,3827.0447,-763.3758,314.87888,NULL,0,0,0,100,0),
(@PATH,22,3836.197,-740.15375,319.43015,NULL,0,0,0,100,0),
(@PATH,23,3841.1611,-713.5064,325.38644,NULL,0,0,0,100,0),
(@PATH,24,3850.8962,-677.3118,328.18048,NULL,0,0,0,100,0),
(@PATH,25,3867.4185,-662.55194,330.74258,NULL,0,0,0,100,0),
(@PATH,26,3893.1277,-649.1434,334.0132,NULL,0,0,0,100,0),
(@PATH,27,3918.8508,-620.3714,339.56857,NULL,0,0,0,100,0),
(@PATH,28,3939.5344,-610.4964,340.5994,NULL,0,0,0,100,0),
(@PATH,29,3967.8643,-599.3677,338.82224,NULL,0,0,0,100,0),
(@PATH,30,3999.6077,-585.43585,338.0969,NULL,0,0,0,100,0);