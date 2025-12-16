# Zöglfrex Reference
## Keys and Shortcuts
### Item keys
- Model: M
- Fixture: F
- Group: G
- Intensity: I
- Color: C
- Position: P
- Raw: R
- Effect: E
- Cuelist: L
- Cue: Q

### Other keys
- . (Period)
- 0
- 1
- 2
- 3
- 4
- 5
- 6
- 7
- 8
- 9
- Attribute: A
- Set: S
- Thru: T

> [!TIP]
> You can also insert a period using the comma key (,).
> This is very useful when inserting IDs on some numeric keypads.

### Keyboard Shortcuts
- Go to next Cue: Space
- Go to previous Cue: Shift + Space
- Select next Group: Arrow down
- Select previous Group: Arrow up
- Select next Fixture in current Group: Arrow right
- Select previous Fixture in current Group: Arrow left
- Deselect the currently selected Fixture: Escape
- Execute command: Return/Enter
- Undo the last keypress in the command input: Backspace
- Clear command input: Shift + Backspace

- Toggle Blind: Shift + B
- Toggle Highlight: Shift + H
- Toggle Solo: Shift + S
- Toggle Skip Fade: Shift + F
- Toggle Tracking: Shift + T
- Scroll through the Modes of the Cuelist View: Shift + M
- Scroll through the Row filters of the Cuelist View: Shift + R

- Quit Zöglfrex: Ctrl + Q

## Item commands
These commands can be executed with all types of Items (Models, Fixtures, Groups, Intensities, Colors, Positions, Raws, Cuelists, Cues).

### Item Attribute 0 Set (Move / Set ID)
This command changes Attribute 0 (the ID) of the selected Item.

Please note that this command only makes sense for one selected Item as two Items of the same type and with the same ID are not allowed.

So, for example for setting the ID of Model 1 to 2, you would need the following command:
```
Model 1 Attribute 0 Set 2
```

### Item Attribute 1 Set (Label)
This command changes Attribute 1 (the Label) of the selected Items, for example:
```
Model 1 + 2 Attribute 1 Set
```
The command will open a popup window where you can insert the Label text for those Models.

This command will copy all the Attributes of Model 1 to Model 2 and 3.

### Item Set - (Delete)
This command deletes Items, for example:
```
Model 1 + 2 Set -
```
This command will remove Model 1 and 2.

> [!CAUTION]
> Please note that deleting Items can affect other Items too:
> - When deleting a Model, the Model Attribute of all Fixtures which used this Model are set to None.
> - When deleting a Model, the Model Exceptions for these Model are deleted in all Attributes.
> - When deleting a Fixture, the Fixture is removed from all Groups.
> - When deleting a Fixture, the Fixture Exceptions for these Fixture are deleted in all Attributes.
> - When deleting a Group, the Group values will be removed from all Cues.
> - When deleting an Intensity, it will be removed from all Effects and Cues.
> - When deleting a Color, it will be removed from all Effects and Cues.
> - When deleting a Position, it will be removed from all Effects and Cues.
> - When deleting a Raw, it will be removed from all Intensities, Colors, Positions, Effects and Cues.
> - When deleting an Effect, it will be removed from all Cues.
> - When deleting a Cuelist, all of its Cues will be deleted.

## Models
If no Model Attribute is given, the standard Attribute 2 will be used.

If no Model ID is given, the Model of the current Fixture will be used.
If no Fixture is currently selected, all the Models of the Fixtures of the current Group will be used instead.

### Model Attribute 2 Set (Channels)
This command doesn't take a value. Instead, it will open a popup where you can insert the Model's channels. These channels are currently supported:
- D (Dimmer)
- d (Dimmer fine)
- R (Red)
- r (Red fine)
- G (Green)
- g (Green fine)
- B (Blue)
- b (Blue fine)
- W (White)
- w (White fine)
- C (Cyan)
- c (Cyan fine)
- M (Magenta)
- m (Magenta fine)
- Y (Yellow)
- y (Yellow fine)
- P (Pan)
- p (Pan fine)
- T (Tilt)
- t (Tilt fine)
- Z (Zoom)
- z (Zoom fine)
- F (Focus)
- f (Focus fine)
- 0 (DMX value 0)
- 1 (DMX value 255)

So, for example, if you want to add a generic RGB Model, you need to type 'RGB' into the popup.

### Model Attribute 3.1 Set (Pan Range)
If your Model supports the Pan Attribute, you should set its Pan Range so that Positions are rendered correctly.
The Pan Range is given in degree.

### Model Attribute 3.2 Set (Tilt Range)
If your Model supports the Tilt Attribute, you should set its Tilt Range so that Positions are rendered correctly.
The Tilt Range is given in degree.

### Model Attribute 4.1 Set (Minimal Zoom)
If your Model supports the Zoom Attribute, you should set its Minimal and Maximal Zoom so that the Zoom can be rendered correctly.
This value is given in degree.

### Model Attribute 4.2 Set (Maximal Zoom)
If your Model supports the Zoom Attribute, you should set its Minimal and Maximal Zoom so that the Zoom can be rendered correctly.
This value is given in degree.

## Fixtures
If no Fixture Attribute is given, the standard Attribute 3 will be used.

If no Fixture ID is given, the current Fixture in the current Group will be used.
If no Fixture is currently selected, all Fixtures of the current Group will be selected.
You can select the current Fixture of the current Group with the left and right arrow keys or like this:
```
Fixture 1
```

### Fixture Attribute 2 Set (Model)
This command sets the Model of the selected Fixtures:
```
Fixture 1 + 2 Attribute 2 Set Model 3
```
Every new Fixture's Model is set to None.
This means that the Fixture won't output any DMX data.

You can also remove the Model from a Fixture, setting the Model Attribute to None again:
```
Fixture 1 Attribute 2 Set -
```

### Fixture Attribute 3.1 Set (Universe)
This command sets the sACN universe of the selected Fixtures.

### Fixture Attribute 3.2 Set (Universe)
This command sets the DMX address of the selected Fixtures.

Every new Fixture is patched to address 0.
This means that it won't output any DMX signal but can still be used for programming.
This is great if you know your Fixture rig but not how it's patched.

### Fixture Attribute 4.1 Set (X Position)
This command sets the X Position of the Fixture in the 2D View.

### Fixture Attribute 4.2 Set (Y Position)
This command sets the Y Position of the Fixture in the 2D View.

### Fixture Attribute 5.1 Set (Rotation)
This command sets the Rotation of the Fixture.

You only need to change this Attribute if your Fixture's pan in the real world is different compared to the one in the 2D View.


### Fixture Attribute 5.2 Set (Invert Pan)
This command is needed if your Fixture's pan rotates into the wrong direction compared with the 2D View.

The accepted values are 0 (False / Pan Normal) and 1 (True / Pan Inverted).

## Groups
If no Group Attribute is given, the standard Attribute 2 will be used.

If no Group ID is given, the ID of the currently selected Group will be used.
You can select the current Group with the up and down arrow keys or like this:
```
Group 1
```

### Group Attribute 2 Set (Fixtures)
This command takes the Fixtures which you want to add to the Group:
```
Group 1 Attribute 2 Set Fixture 1.1 + 1.2 + 1.3
```
You can also give no Fixture IDs which results in an empty Group:
```
Group 1 Attribute 2 Set -
```

## Intensities
If no Itensity Attribute is given, the standard Attribute 2 will be used.

If no Intensity ID is given, the ID of the Intensity of the currently selected Group in the currently selected Cue will be used.

### Intensity Attribute 2 Set (Dimmer)
This command sets the Dimmer Attribute to the selected Intensities:
```
Intensity 1 + 2 Attribute 2 Set 75
```
Please note that the value is given in percent.

You can also define exception values for Models and Fixtures:
```
Intensity 1 + 2 Attribute 2 Model 7 Set 80
Intensity 1 + 2 Attribute 2 Fixture 5 Set 70
```
These values will replace the standard values if the Fixtures or the Models match.
You can also remove the exception values again:
```
Intensity 1 + 2 Attribute 2 Model 7 Set -
Intensity 1 + 2 Attribute 2 Fixture 5 Set -
```

It is also possible to set the difference of the Intensity:
```
Intensity 1 Attribute 2 Set + 10
Intensity 2 Attribute 2 Set + - 5
```
The first command will increase the Dimmer value of Intensity 1 by 10 percent.
The second command will reduce the Dimmer value of Intensity 2 by 5 percent.

> [!CAUTION]
> Please note that this command won't work as it tries to set the Dimmer to -10 percent instead of reducing it by 10 percent:
> ```
> Intensity 2 Attribute 2 Set - 10
> ```

### Intensity Attribute 3 Set (Raws)
This command allows you to set Raws which will be called together with the Intensity:
```
Intensity 1 Attribute 3 Set Raw 1 + 2 + 3
```
For example, you could use this Attribute to call a Raw which controls the shutter of a Moving Head.

You can remove all Raws again like this:
```
Intensity 1 Attribute 3 Set -
```

You can also add Raws to the Intensity without overwriting all the Raws like this:
```
Intensity 1 Attribute 3 Set + Raw 4
```

## Colors
If no Color Attribute is given, the standard Attribute 2 will be used.

If no Color ID is given, the ID of the Color of the currently selected Group in the currently selected Cue will be used.

### Color Attribute 2 Set (Hue)
This command sets the Hue Attribute of the selected Colors.
The value is given in degree.

For this Attribute, you can also give Model and Fixture exceptions or set a difference, just like with Intensity Attribute 2.

### Color Attribute 3 Set (Saturation)
This command sets the Saturation of the selected Colors.
The value is given in percent.

For this Attribute, you can also give Model and Fixture exceptions or set a difference, just like with Intensity Attribute 2.

### Color Attribute 4 Set (Quality)
This command sets the quality of the selected Colors.
The value is given in percent:
```
Color 1 Attribute 4 Set 75
```

This is important in combination with RGBW color mixing:
The color white can be producted with only the white LEDs turned on (Quality = 100%) or with all LEDs turned on (Quality = 0%).

For this Attribute, you can also give Model and Fixture exceptions or set a difference, just like with Intensity Attribute 2.

### Color Attribute 5 Set (Raws)
This command allows you to set Raws which will be called together with the Color:
```
Color 1 Attribute 5 Set Raw 1 + 2 + 3
```
For example, you could use this Attribute to call a Raw which controls the color wheel of a Moving Head.

You can remove all Raws again like this:
```
Color 1 Attribute 5 Set -
```

You can also add Raws to the Color without overwriting all the Raws like this:
```
Color 1 Attribute 5 Set + Raw 4
```

## Positions
If no Position Attribute is given, the standard Attribute 2 will be used.

If no Position ID is given, the ID of the Position of the currently selected Group in the currently selected Cue will be used.

### Position Attribute 2 Set (Pan)
This command sets the Pan Attribute of the selected Positions.
The value is given in degree.

For this Attribute, you can also give Model and Fixture exceptions or set a difference, just like with Intensity Attribute 2.

### Position Attribute 3 Set (Tilt)
This command sets the Tilt of the selected Positions.
The value is given in degree.

For this Attribute, you can also give Model and Fixture exceptions or set a difference, just like with Intensity Attribute 2.

### Position Attribute 4 Set (Zoom)
This command sets the Zoom of the selected Positions.
The value is given in degree.

For this Attribute, you can also give Model and Fixture exceptions or set a difference, just like with Intensity Attribute 2.

### Position Attribute 5 Set (Focus)
This command sets the Focus of the selected Positions.
The value is given in percent.

For this Attribute, you can also give Model and Fixture exceptions or set a difference, just like with Intensity Attribute 2.

### Position Attribute 6 Set (Raws)
This command allows you to set Raws which will be called together with the Position:
```
Position 1 Attribute 5 Set Raw 1 + 2 + 3
```
For example, you could use this Attribute to call a Raw which controls the iris of a Moving Head.

You can remove all Raws again like this:
```
Position 1 Attribute 5 Set -
```

You can also add Raws to the Position without overwriting all the Raws like this:
```
Position 1 Attribute 5 Set + Raw 4
```

## Raws
If no Raw Attribute is given, the standard Attribute 2 will be used.
If no Raw IDs are given, the IDs of the Raws of the currently selected Group in the current Cue will be used.

### Raw Attribute 2.x Set (Channel Values)
This command sets the value of channel x in the selected Raws:
```
Raw 1 + 2 Attribute 2.7 Set 213
```
This example would set channel 7 of the Fixtures the Raw is applied to to the DMX value 213.

The channel x has to be between 1 and 512.
The value has to be between 0 and 255.

You can also give value exceptions for Models or Fixtures:
```
Raw 1 Attribute 2.4 Fixture 3 Set 200
Raw 2 Attribute 2.6 Fixture 5 Set 150
```
It is also possible to remove one channel or all channels of a Fixture or Model again:
```
Raw 1 Attribute 2 Model 4 Set -
Raw 2 Attribute 2.7 Fixture 5 Set -
```

### Raw Attribute 3 Set (Move while Dark)
This command sets if the Raw channels are allowed to be set before the actual Cue where they are requested takes place.

The accepted values are 0 (False / Move while Dark deactivated) and 1 (True / Move while Dark activated).

### Raw Attribute 4 Set (Fade)
This command sets if the Raw is allowed to fade the Channels.

The accepted values are 0 (False / Fading deactivated) and 1 (True / Fading activated).

## Effects
If no Effect Attribute is given, the standard Attribute 2 will be used.
If no Effect IDs are given, the IDs of the Effects of the currently selected Group in the current Cue will be used.

### Effect Attribute 2 Set (Steps)
This command sets the amount of steps of the Effect.

### Effect Attribute 3.x Set (Intensities)
This command sets the Intensity of Step x in the selected Effects:
```
Effect 1 Attribute 3.1 Set Intensity 1
```
You can remove the Intensity like this:
```
Effect 1 Attribute 3.1 Set -
```
Please note that as soon as at least one Effect step holds an Intensity value, the Effect will overwrite the Intensity.

### Effect Attribute 4.x Set (Colors)
This command sets the Color of Step x in the selected Effects:
```
Effect 1 Attribute 4.1 Set Color 1
```
You can remove the Color like this:
```
Effect 1 Attribute 4.1 Set -
```
Please note that as soon as at least one Effect step holds a Color value, the Effect will overwrite the Color.

### Effect Attribute 5.x Set (Positions)
This command sets the Position of Step x in the selected Effects:
```
Effect 1 Attribute 5.1 Set Position 1
```
You can remove the Position like this:
```
Effect 1 Attribute 5.1 Set -
```
Please note that as soon as at least one Effect step holds a Position value, the Effect will overwrite the Position.

### Effect Attribute 6.x Set (Raws)
This command sets the Raws of Step x in the selected Effects:
```
Effect 1 Attribute 6.1 Set Raw 1 + 2
```
You can remove the Raws like this:
```
Effect 1 Attribute 6.1 Set -
```
Please note that as soon as at least one Effect step holds a Raw, the Effect will overwrite every Channel of the Raw.

### Effect Attribute 7 Set (Hold)
This command sets the Hold time of the Effect steps:
```
Effect 1 Attribute 7 Set 1.5
```
The value is given in seconds.

You can also set and remove Step exceptions like this:
```
Effect 1 Attribute 7.2 Set 2
Effect 1 Attribute 7.1 Set -
```

### Effect Attribute 8 Set (Fade)
This command sets the Fade time of the Effect steps:
```
Effect 1 Attribute 8 Set 1.5
```
The value is given in seconds.

You can also set and remove Step exceptions like this:
```
Effect 1 Attribute 8.2 Set 2
Effect 1 Attribute 8.1 Set -
```

### Effect Attribute 9 Set (Phase)
This command sets the standard Phase the Effects starts with:
```
Effect 1 Attribute 9 Set 180
```
The value is given in degree.

You can also set Fixture exceptions in order to produce phase differences between Fixtures:
```
Effect 1 Attribute 9 Fixture 4 Set 90
```
You can remove the Fixture exceptions again like this:
```
Effect 1 Attribute 9 Fixture 4 Set -
```

### Raw Attribute 10 Set (Sine Fade)
This command sets if the Fades are rendered as linear transitions or if the are shaped like a sine wave.

The accepted values are 0 (False / Linear Fade) and 1 (True / Sine-Shaped Fade).

## Cuelists
If no Cuelist Attribute is given, the standard Attribute 2 will be used.

If no Cuelist ID is given, the ID of the current Cuelist will be used.
You can select the current Cuelist like this:
```
Cuelist 1
```

### Cuelist Attribute 2 Set (Priority)
This command sets the Priority of a Cuelist.
This is a number between 0 and 200.

If multiple Cuelists try to control the same Fixture, the Cuelist with the highest ID will win.

### Cuelist Attribute 3 Set (Move while Dark)
This command sets if the Fixtures which are not set are allowed to be set before the actual Cue where they are requested takes place.

The accepted values are 0 (False / Move while Dark deactivated) and 1 (True / Move while Dark activated).

> [!CAUTION]
> Please note that Move while Dark will only look at the next 10 Cues in each Cuelist in order to reduce the required processing.

## Cues
You can only edit Cues of the currently selected Cuelist.
All Cues must be member of one Cuelist.

If no Cue Attribute is given, the standard Attribute 8 will be used.

If no Cue ID is given, the ID of the selected Cue of the selected Cuelist will be used.
You can select a Cue like this:
```
Cue 1
```
Please note that when you add a new Cue, the Cue will use the values of Attribute 2, 3, 4, 5 and 6 of the previouse Cue in the Cuelist as these values will be tracked.

### Cue Attribute 2 Set (Intensities)
This command sets the Intensity of the selected Groups in the selected Cues:
```
Cue 1 + 2 Attribute 2 Group 4 Set Intensity 3
```
You can remove the Intensity like this:
```
Cue 1 + 2 Attribute 2 Group 4 Set -
```

> [!TIP]
> You can set the Intensity of the currently selected Group in the current Cue like this:
> ```
> Intensity 1
> ```
> You can also remove the Intensity of the currently selected Group in the current Cue like this:
> ```
> Intensity -
> ```
> If you use this syntax while Tracking is enabled, the Intensity will be tracked in the next Cues until a Blocked Cue or a Cue where the Group holds a different Intensity.

### Cue Attribute 3 Set (Colors)
This command sets the Color of the selected Groups in the selected Cues:
```
Cue 1 + 2 Attribute 3 Group 4 Set Color 3
```
You can remove the Color like this:
```
Cue 1 + 2 Attribute 3 Group 4 Set -
```

> [!TIP]
> You can set the Color of the currently selected Group in the current Cue like this:
> ```
> Color 1
> ```
> You can also remove the Color of the currently selected Group in the current Cue like this:
> ```
> Color -
> ```
> If you use this syntax while Tracking is enabled, the Color will be tracked in the next Cues until a Blocked Cue or a Cue where the Group holds a different Color.

### Cue Attribute 4 Set (Positions)
This command sets the Position of the selected Groups in the selected Cues:
```
Cue 1 + 2 Attribute 4 Group 4 Set Position 3
```
You can remove the Position like this:
```
Cue 1 + 2 Attribute 4 Group 4 Set -
```

> [!TIP]
> You can set the Position of the currently selected Group in the current Cue like this:
> ```
> Position 1
> ```
> You can also remove the Position of the currently selected Group in the current Cue like this:
> ```
> Position -
> ```
> If you use this syntax while Tracking is enabled, the Position will be tracked in the next Cues until a Blocked Cue or a Cue where the Group holds a different Position.

### Cue Attribute 5 Set (Raws)
This command sets the Raws of the selected Groups in the selected Cues:
```
Cue 1 + 2 Attribute 5 Group 3 Set Raw 5 + 6
```
You can remove all Raws like this:
```
Cue 1 + 2 Attribute 5 Group 3 Set -
```
> [!TIP]
> You can set the Raws of the currently selected Group in the current Cue like this:
> ```
> Raw 1 + 2 + 3
> ```
> You can also remove the Raws of the currently selected Group in the current Cue like this:
> ```
> Raw -
> ```
> If you use this syntax while Tracking is enabled, the Raws will be tracked in the next Cues until a Blocked Cue or a Cue where the Group holds different Raws.

### Cue Attribute 6 Set (Effects)
This command sets the Effects of the selected Groups in the selected Cues:
```
Cue 1 + 2 Attribute 6 Group 3 Set Effect 5 + 6
```
You can remove all Effects like this:
```
Cue 1 + 2 Attribute 6 Group 3 Set -
```
> [!TIP]
> You can set the Effects of the currently selected Group in the current Cue like this:
> ```
> Effect 1 + 2 + 3
> ```
> You can also remove the Effects of the currently selected Group in the current Cue like this:
> ```
> Effect -
> ```
> If you use this syntax while Tracking is enabled, the Effects will be tracked in the next Cues until a Blocked Cue or a Cue where the Group holds different Effects.

### Cue Attribute 7 Set (Block)
This command sets if the selected Cues will be blocked Cues.
This means that tracked values will not pass on to those Cues.
```
Cue 1 + 2 Attribute 7 Set 1
```
The accepted values are 0 (False / Block deactivated) and 1 (True / Block activated).

### Cue Attribute 8 Set (Fade)
This command sets the Fade time of the selected Cues:
```
Cue 1 Attribute 8 Set 17.4
```
The value is given in seconds.

For this Attribute, you can also give Fixture exceptions or set a difference, just like with Intensity Attribute 2.

### Cue Attribute 9 Set (Delay)
This command sets the Delay time of the selected Cues:
```
Cue 1 Attribute 9 Set 17.4
```
The value is given in seconds.

For this Attribute, you can also give Fixture exceptions or set a difference, just like with Intensity Attribute 2.

### Cue Attribute 10 Set (Follow)
This command sets if this Cue will directly follow after the Fade of the previous Cue has ended:
```
Cue 1 Attribute 10 Set 1
```

The accepted values are 0 (False / Follow deactivated) and 1 (True / Follow activated).

### Cue Attribute 11 Set (Sine Fade)
This command sets if the Fades are rendered as linear transitions or if the are shaped like a sine wave.

The accepted values are 0 (False / Linear Fade) and 1 (True / Sine-Shaped Fade).

## Selecting Items
There are some useful tricks for selecting Items using IDs.
You can use these whenever you can enter multiple IDs.

### Combining IDs with +
You can combine multiple IDs with +:
```
Intensity 1 + 2.1 + 1.7 Set -
```

### Combining IDs with -
You can remove IDs from the selection like this:
```
Intensity 1.1 Thru .8 - 1.4 Thru .7 Set -
```
This command will only remove the IDs 1.1, 1.2, 1.3 and 1.8.

### IDs ending with .
This syntax is useful when you want to select all IDs with the same beginning.
However, please note that this command can only select already existing IDs, so you can't use it for Items which don't exist yet!
If they existed, this command would select Intensities 1, 1.1 or any other Intensity with an ID starting with a '1.'.
```
Intensity 1. Set -
```

### All Items with .
You can simply select all Items using a period.
This command, for example, deletes all existing intensities:
```
Intensity . Set -
```

### Selecting multiple Items with Thru .
You can also select multiple IDs with Thru.
This command would select the Intensity IDs 1.9, 1.10 and 1.11.
```
Intensity 1.9 Thru .11 Set -
```
This command deleted the Intensity IDs 1, 2, 3:
```
Intensity 1 Thru .3 Set -
```

### Selecting multiple Items with Thru
You can also select all Items in a range like this:
```
Intensity 1.1 Thru 2.3 Set -
```
This command deletes all Intensities starting with Intensity 1.1 and ending with Intensity 2.3.

## Control Panel
The Control panel can be used to control the values of the current Group or Fixture:

When selecting a Cue and a Group, you can see and change the Dimmer of the current Intensity, the Hue and Saturation of the current Color and the Pan, Tilt, Zoom and Focus of the current Position.
When you now also select a Fixture which has defined an exception for the Attribute, not the standard value but instead the value for this Fixture will be changed.
So, for example, if your current Intensity holds a Dimmer exception for the current Fixture, rotating the Dial will change this exception.
You can also add and remove exceptions by clicking the exception buttons.

In the Raws tab, you can select a Raw and set Channel values.
Here, you can also add and remove values and exceptions by clicking the exception buttons.

> [!CAUTION]
> Please note that the precision of the dials cannot be smaller than 1.
> When you need a more precise control, you can of course still set the values with the Terminal.
