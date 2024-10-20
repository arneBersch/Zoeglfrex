## Keys and Shortcuts
### Item keys
- Model: M
- Fixture: F
- Group: G
- Intensity: I
- Color: C
- Transition: T
- Row: R
- Cue: Q

### Other keys
- . (Period)
- , (Comma)
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

### Operator keys
- Record: Shift + R
- Label: Shift + L
- Move: Shift + M
- Copy: Shift + C
- Delete: Shift + D

### Keyboard Shortcuts
- Execute command and clear the Terminal (if the command succeeds): Return / Enter
- Execute command without clearing the Terminal: Shift + Return / Shift + Enter
- New File: Control + N
- Open File: Control + O
- Save File: Control + S
- Save File as: Control + Shift + S
- Quit Zöglfrex: Control + Q

## Models
### Model Record
Model Record doesn't take any parameters. Instead, it will open a popup where you can insert the Model's Channels. These channels are currently supported:
- Dimmer
- Red
- Green
- Blue

So, for example, if you want to add a generic RGB Model at ID 3.1, you have to first have to execute this:
```
Model 1 Record
```
And after this, you need to type RGB into the popup.
### Model Label
Model Label doesn't take any parameters. Instead, it will open a popup where you can insert the new label of the selected Models.
### Model Move
Model Move takes the ID you want to move the selected Model to as a parameter.
> [!NOTE]
> Please note that you can currently move only one Model as the ID where you move the Fixtures to will be taken after the first moved Model.
### Model Copy
Model Copy takes the ID you want to copy the selected Model to as a parameter.
> [!NOTE]
> Please note that you can currently copy only one Model as the ID where you copy the Fixtures to will be taken after the first copied Model.
### Model Delete
Model Delete doesn't take any parameters.
It deletes the selected Models.
> [!CAUTION]
> Please note that Model Delete will also delete all Fixtures which are linked with the selected Models.

## Fixtures
### Fixture Record
Fixture Record takes either the Model of the new Fixture, its DMX Address or both.
You need to insert the Address first and then you can insert Model and the Model ID:
```
Fixture 1 Record
```
Please note that you have to give a Model when recording a new Fixture.
### Fixture Label
Fixture Label doesn't take any parameters.
Instead, it will open a popup where you can insert the new label of the selected Fixtures.
### Fixture Move
Fixture Move takes the ID you want to move the selected Fixture to as a parameter.
> [!NOTE]
> Please note that you can currently move only one Fixture as the ID where you move the Fixtures to will be taken after the first moved Fixture.
### Fixture Copy
Fixture Copy takes the ID you want to copy the selected Fixture to as a parameter.
> [!NOTE]
> Please note that you can currently copy only one Fixture as the ID where you copy the Fixtures to will be taken after the first copied Fixture.
### Fixture Delete
Fixture Delete doesn't take any parameters.
It deletes the selected Fixtures.
> [!CAUTION]
> Please note that Fixture Delete will also remove the Fixture from every Group.

## Groups
### Group Record
Group Record takes the Fixtures which you want to add to the Group:
```
Group 1 Record Fixture 1.1, 1.2, 1.3
```
You can also give no Fixture IDs which would result in an empty Group.
> [!WARNING]
> Please note that Group Record will first overwrite the Fixture which were in the Group before Group Record was executed.
### Group Label
Group Label doesn't take any parameters.
Instead, it will open a popup where you can insert the new label of the selected Groups.
### Group Move
Group Move takes the ID you want to move the selected Group to as a parameter.
> [!NOTE]
> Please note that you can currently move only one Group as the ID where you move the Group to will be taken after the first moved Group.
### Group Copy
Group Copy takes the ID you want to copy the selected Group to as a parameter.
> [!NOTE]
> Please note that you can currently copy only one Group as the ID where you copy the Groups to will be taken after the first copied Group.
### Group Delete
Group Delete doesn't take any parameters.
It deletes the selected Groups.
> [!CAUTION]
> Please note that Group Delete will also delete all Rows which are linked to the selected Group.

## Intensities
### Intensity Record
Intensity Record takes the Dimmer value of the Intensity in percent as an argument.
### Intensity Label
Intensity Label doesn't take any parameters.
Instead, it will open a popup where you can insert the new label of the selected Intensities.
### Intensity Move
Intensity Move takes the ID you want to move the selected Intensity to as a parameter.
> [!NOTE]
> Please note that you can currently move only one Intensity as the ID where you move the Intensity to will be taken after the first moved Intensity.
### Intensity Copy
Intensity Copy takes the ID you want to copy the selected Intensity to as a parameter.
> [!NOTE]
> Please note that you can currently copy only one Intensity as the ID where you copy the Intensity to will be taken after the first copied Intensity.
### Intensity Delete
Intensity Delete doesn't take any parameters.
It deletes the selected Intensities.
> [!CAUTION]
> Please note that Intensity Delete will also remove the selected Intensities from all Cues.

## Colors
### Color Record
Color Record takes the rdd, green and blue values of the Color in percent as arguments.
For example, you can add a new Color 3.1 with 12.3% red, 45.6% green and 78.9% blue like this:
```
Color 1 Record 12.3,45.6,78.9
```
You can also leave out values if you don't want to change them, for example this command will only set the green value to 45.6 percent:
```
Color 1 Record ,45.6
```
### Color Label
Color Label doesn't take any parameters.
Instead, it will open a popup where you can insert the new label of the selected Colors.
### Color Move
Color Move takes the ID you want to move the selected Color to as a parameter.
> [!NOTE]
> Please note that you can currently move only one Color as the ID where you move the Color to will be taken after the first moved Color.
### Color Copy
Color Copy takes the ID you want to copy the selected Color to as a parameter.
> [!NOTE]
> Please note that you can currently copy only one Color as the ID where you copy the Color to will be taken after the first copied Color.
### Color Delete
Color Delete doesn't take any parameters.
It deletes the selected Colors.
> [!CAUTION]
> Please note that Color Delete will also remove the selected Colors from all Cues.

## Transitions
### Transition Record
Transition Record takes the fade time value of the Transition in seconds as an argument.
### Transition Label
Transition Label doesn't take any parameters.
Instead, it will open a popup where you can insert the new label of the selected Transitions.
### Transition Move
Transition Move takes the ID you want to move the selected Transition to as a parameter.
> [!NOTE]
> Please note that you can currently move only one Transition as the ID where you move the Transition to will be taken after the first moved Transition.
### Transition Copy
Transition Copy takes the ID you want to copy the selected Transition to as a parameter.
> [!NOTE]
> Please note that you can currently copy only one Transition as the ID where you copy the Transition to will be taken after the first copied Transition.
### Transition Delete
Transition Delete doesn't take any parameters.
It deletes the selected Transitions.
> [!CAUTION]
> Please note that Transition Delete will also remove all Cues linked to the selected Transitions.

## Rows
### Row Record
Row Record takes the Group of the Row as an argument:
```
Row 1 Record Group 1
```
### Row Label
Row Label doesn't take any parameters.
Instead, it will open a popup where you can insert the new label of the selected Rows.
### Row Move
Row Move takes the ID you want to move the selected Row to as a parameter.
> [!NOTE]
> Please note that you can currently move only one Row as the ID where you move the Row to will be taken after the first moved Row.
### Row Copy
Row Copy takes the ID you want to copy the selected Row to as a parameter.
> [!NOTE]
> Please note that you can currently copy only one Row as the ID where you copy the Row to will be taken after the first copied Row.
### Row Delete
Row Delete doesn't take any parameters.
It deletes the selected Rows.

## Cues
### Cue Record
Cue Record takes either the Transition or Row values as parameters:
```
Cue 1 Record Transition 1
Cue 1 Record Row 1 Intensity 1 Color 1
```
Please note that you have to record a Transition first when adding a new Cue.
When recording a value for a Row, you can also give only an Intensity or only a Color.
### Cue Label
Cue Label doesn't take any parameters.
Instead, it will open a popup where you can insert the new label of the selected Cues.
### Cue Move
Cue Move takes the ID you want to move the selected Cue to as a parameter.
> [!NOTE]
> Please note that you can currently move only one Cue as the ID where you move the Cue to will be taken after the first moved Cue.
### Cue Copy
Cue Copy takes the ID you want to copy the selected Cue to as a parameter.
> [!NOTE]
> Please note that you can currently copy only one Cue as the ID where you copy the Cue to will be taken after the first copied Cue.
### Cue Delete
Cue Delete takes either no parameters or a Row as an parameter.
When no parameter is given, it will delete the selected Cues.
If a Row is given, it will clear the Intensity and Color of the Row in the selected Cues.
> [!TIP]
> You can also delete only the Intensity or Color of a Row: 
> ```
> Cue 1 Delete Row 1 Intensity
> ```

## sACN Output:
Zöglfrex sends sACN...
- on port 5568
- on IP 239.255.0.1
- on sACN universe 1
- with a frequency of 40 Hz (one Sequence every 25ms)
- using priority 100
