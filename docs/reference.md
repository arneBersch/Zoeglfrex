## Keys and Shortcuts
### Item keys
- Model: M
- Fixture: F
- Group: G
- Intensity: I
- Color: C
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
- T (Thru)

> [!TIP]
> You can also insert a period using the comma key (,).
> This is very useful when inserting IDs using a numeric keypad.

### Operator keys
- Record: Shift + R
- Label: Shift + L
- Move: Shift + M
- Copy: Shift + C
- Delete: Shift + D

### Keyboard Shortcuts
- Execute command and clear the Terminal: Return / Enter
- Execute command without clearing the Terminal: Shift + Return / Shift + Enter
- New File: Control + N
- Open File: Control + O
- Save File: Control + S
- Save File as: Control + Shift + S
- Quit Zöglfrex: Control + Q

## Models
### Model Record
Model Record doesn't take any parameters. Instead, it will open a popup where you can insert the Model's Channels. These channels are currently supported:
- D (Dimmer)
- R (Red)
- G (Green)
- B (Blue)
- 0 (DMX always DMX value 0)
- 1 (DMX always DMX value 255)

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
> Please note that you can currently move only one Model as the ID where you move the Model to will be taken after the first moved Model.
### Model Copy
Model Copy takes the ID you want to copy the selected Model to as a parameter.
> [!NOTE]
> Please note that you can currently copy only one Model as the ID where you copy the Model to will be taken after the first copied Model.
### Model Delete
Model Delete doesn't take any parameters.
It deletes the selected Models.
> [!CAUTION]
> Please note that Model Delete will also delete all Fixtures which are linked with the selected Models.

## Fixtures
### Fixture Record
Fixture Record takes either the Model of the new Fixture or its DMX address:
```
Fixture 1 Record Model 7
Fixture 1 Record 25
```
Please note that you have to give a Model when recording a new Fixture.
### Fixture Label
Fixture Label doesn't take any parameters.
Instead, it will open a popup where you can insert the new label of the selected Fixtures.
### Fixture Move
Fixture Move takes the ID you want to move the selected Fixture to as a parameter.
> [!NOTE]
> Please note that you can currently move only one Fixture as the ID where you move the Fixture to will be taken after the first moved Fixture.
### Fixture Copy
Fixture Copy takes the ID you want to copy the selected Fixture to as a parameter.
> [!NOTE]
> Please note that you can currently copy only one Fixture as the ID where you copy the Fixture to will be taken after the first copied Fixture.
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
You can also give no Fixture IDs which would result in an empty Group:
```
Group 1 Record
```
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
> Please note that you can currently copy only one Group as the ID where you copy the Group to will be taken after the first copied Group.
### Group Delete
Group Delete doesn't take any parameters.
It deletes the selected Groups.

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
Color 1 Record 12.3 + 45.6 + 78.9
```
You can also leave out values if you don't want to change them, for example this command will only set the green value to 45.6 percent:
```
Color 1 Record +45.6
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

## Cues
### Cue Record
Cue Record takes either the fade time or Group values as parameters:
```
Cue 1 Record 25
Cue 1 Record Group 1 Intensity 1 Color 1
```
Please note that you have to record a fade time first when adding a new Cue.
When recording a value for a Group, you can also give only an Intensity or only a Color.
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
Cue Delete takes either no parameters or a Group as an parameter.
When no parameter is given, it will delete the selected Cues.
If a Group is given, it will clear the Intensity and Color of the Group in the selected Cues:
```
Cue 1 Delete Group 2
```
> [!TIP]
> You can also delete only the Intensity or Color of a Group: 
> ```
> Cue 1 Delete Group 1 Intensity
> ```

## sACN Output:
Zöglfrex sends sACN...
- on port 5568
- on IP 239.255.0.1
- on sACN universe 1
- with a frequency of 40 Hz (one Sequence every 25ms)
- using priority 100

## Selecting Items
There are some useful tricks for selecting items using IDs.
You can use these whenever you can enter multiple IDs.

### Combining IDs with +
You can combine multiple IDs with +:
```
Intensity 1 + 2.1 + 1.7 Record 100
```

### IDs ending with .
This syntax is useful when you want to select all IDs with the same beginning.
However, please note that this command can only select already existing IDs, so you can't use it for recording items which don't exist yet!
If they existed, this command would select Intensities 1, 1.1 or any other Intensity with an ID starting with a '1'.
```
Intensity 1. Delete
```

### All items with .
You can simply select all items using a period.
This command, for example, deletes all existing intensities:
```
Intensity . Delete
```

### Selecting multiple items with Thru .
You can also select multiple IDs with Thru.
This command would select the Intensity IDs 1.9, 1.10 and 1.11.
```
Intensity 1.9 Thru .11 Record 100
```
This command selects the Intensity IDs 1, 2, 3:
```
Intensity 1 Thru .3 Record 100
```
