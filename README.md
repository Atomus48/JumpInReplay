# JumpInReplay
JumpInReplay is a bakkesmod Plugin which allows you to open replays in a private match and take control of any car in any situation

**Basic Usage:**

1. open a replay
2. open bakkesmod window (F2)
3. go to the plugins tab and select JumpInReplay
4. press the apply standard bindings button
5. press the convert Replay button
6. wait until the replay is finished
7. a private match will automatically be started and your team will be automatically be selected
8. the JumpInReplay will start and replay the Replay
9. you can now take control by pressing the button that is bound to JumpIn (Back/Select/V)
10. by pressing pause (LeftStickPress/B) you can reset the shot and pause the replay
11. you can close the replay anytime by just leaving the game
12. you can reopen the previously saved replay by pressing the open Replay button in the bakkesmod window (F2)

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
