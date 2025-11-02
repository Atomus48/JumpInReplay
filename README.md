# JumpInReplay
JumpInReplay is a bakkesmod Plugin which allows you to open replays in a private match and take control of any car in any situation

## Installation from git (bakkesmod plugin page is not updated yet)
1. Download the plugin's .zip file from the repo.
2. Locate the .dll file within the downloaded JumpInReplay\plugins folder.
3. Copy the .dll file to your BakkesMod plugins folder: 
   - C:\Users\<username>\AppData\Roaming\bakkesmod\bakkesmod\plugins
4. Locate the settings file within the downloaded JumpInReplay\plugins\settings folder.
5. Copy the settings file to your BakkesMod settings folder:
   - C:\Users\<username>\AppData\Roaming\bakkesmod\bakkesmod\plugins\settings
6. Enable Automatic Startup by modifying your BakkesMod configuration file:
   - Open the plugins.cfg file located at: C:\Users\<username>\AppData\Roaming\bakkesmod\bakkesmod\cfg
   - Add the following line to the end of the file:
     ```
     plugin load jumpinreplay
     ```
   - Save and close the plugins.cfg file.
7. Start Rocket League and enjoy!

**Bindings:**

Standard bindings are:
- `DPadRight/RightArrow`: skips 10s forward in Replay.
- `DPadLeft/LeftArrow`: skips 5s back in Replay.
- `DPadUp/UpArrow`: selects spectated Player.
- `DPadDown/DownArrow`: selects previously spectated Player.
- `LeftStickPress/B`: pauses Replay (also resets shot).
- `Back/Select/V`: toggles JumpInMode (you have to unpause or execute an input like custom training).

if you want to change the bindings you can do it in the bakkesmod bindings tab.

**Command Reference:**

- `jumpIn_convert`: converts replay you are currently watching into JumpInReplay.
- `jumpIn_autoConvert`: automatically converts replay when opening replay.
- `jumpIn_replaySave`: saves the replay you are currently watching as a JumpInReplay.
- `jumpIn_openReplay`: opens the most recently converted replay.
- `jumpIn_bindings`: applys standard bindings for JumpInReplay.
- `jumpIn_resolution`: changes the resolution the replay is converted with (higher = faster but worse quality).
- `jumpIn_limitedBoost`: sets boost to be limited or unlimited in the JumpInReplay.
- `jumpIn_inputToUnpause`: allows you to unpause the JumpInReplay by throttle or boost when JumpedIn.
- `jumpIn_showHud`: enables the HUD in JumpInReplays.
- `jumpIn_jumpIn`: lets you take control of the player you are currently spectating in a JumpInReplay.
- `jumpIn_pause`: pauses the replay (also resets the shot if you are JumpedIn).
- `jumpIn_skip`: skips x second in the replay (negative x = skips back).
- `jumpIn_switchPlayer`: changes player you are currently spectating.
- `jumpIn_switchBack`: changes player back to the player you were previously spectating.
- `jumpIn_convertKeyframes`: only converts Keyframes of the replay that are previously put in.
- `jumpIn_timeBeforKeyframe`: time that is converted before a Keyframe in seconds.
- `jumpIn_timeAfterKeyframe`: time that is converted after a Keyframe in seconds.
- `jumpIn_disableGoal`: EXPERIMENTAL disabling might cause game to crash.
- `jumpIn_doNotAskForDisableOfIncompatiblePlugins`: doesn't ask before disabling incompatible plugins.

**Currently not Supported:**

- games where players leave (results in ghost cars)
- rumble, dropshot and most limited time gamemodes
- bumping or demoing of bots
- real car layouts of bots
- realistic boost pads on field

**Contact:**

feel free to give me suggestions and report bugs via discord: Atomus#5492 or github.
