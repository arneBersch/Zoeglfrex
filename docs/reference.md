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
- Attribute: A
- Set: S
- Thru: T

> [!TIP]
> You can also insert a period using the comma key (,).
> This is very useful when inserting IDs using a numeric keypad.

### Keyboard Shortcuts
- Execute command and clear the Terminal: Return/Enter
- Execute command without clearing the Terminal: Shift + Return/Enter
- Undo the last keypress in the command input: Backspae
- Clear command input: Shift + Backspace
- New File: Ctrl + N
- Open File: Ctrl + O
- Save File: Ctrl + S
- Save File as: Ctrl + Shift + S
- Quit Zöglfrex: Ctrl + Q

## Item commands
These commands can be executed with all types of Items (Models, Fixtures, Groups, Intensities, Colors, Cues).

### Item Attribute 0 Set (Move / Set ID)
This command changes Attribute 0 (the ID) of the selected Item.
Please note that this command only makes sense for one selected Item as two Items with the same type and the same ID are not allowed.

So, for example for setting the ID of Model 1 to 2, you would need the following command:
```
Model 1 Attribute 0 Set 2
```

### Item Attribute 1 Set (Label)
This command changed Attribute 1 (the Label) of the selected Items, for example:
```
Model 1 + 2 Attribute 1 Set
```
This command will open a popup window where you can insert the Label text for those Models.

### Item Set Item
This command can be used for copying Items, for example:
```
Model 2 + 3 Set Model 1
```
This command will copy the Attributes of Model 1 to Model 2 and 3.

> [!NOTE]
> Please note that when copying Fixtures, the ID of the Fixture won't be copied as this would result in a DMX address conflict.

### Item Set - (Delete)
This command deletes Items, for example:
```
Model 1 + 2 Set -
```
This command will remove Model 1 and 2.

> [!CAUTION]
> Please note that deleting Items can affect other Items to:
> - When deleting a Model, the Model Attribute of all Fixtures which used this Model are set to None (Dimmer).
> - When deleting a Fixture, the Fixture is removed from all Groups.
> - When deleting a Group, the Group will be removed from all Cues.
> - When deleting an Intensity or Color, it will be removed from all Cues

## Models
### Model Attribute 2 Set (channels)
Model Attribute 2 Set doesn't take a value. Instead, it will open a popup where you can insert the Model's channels. These channels are currently supported:
- D (Dimmer)
- R (Red)
- G (Green)
- B (Blue)
- C (Cyan)
- M (Magenta)
- Y (Yellow)
- 0 (DMX always DMX value 0)
- 1 (DMX always DMX value 255)

So, for example, if you want to add a generic RGB Model, you have to first need to type 'RGB' into the popup.

## Fixtures
### Fixture Attribute 2 Set
Fixture Attribute 2 Set sets the Model of the selected Fixtures:
```
Fixture 1 + 2 Attribute 2 Set Model 3
```
Every new Fixture's Model is set to None.
This means that the Fixture is a simple Dimmer.
You can also remove the Model from a Fixture, setting the Model Attribute to None again:
```
Fixture 1 Attribute 2 Set -
```


## Fixture Attribute 3 Set
Fixture Attribute 3 Set sets the DMX address of the selected Fixtures.

Every new Fixture is patched to address 0.
This means that it won't output any DMX signal but can still be used for programming.
This is great if you know your Fixture rig but not how it's patched.

## Groups
### Group Attribute 2 Set
Group Attribute 2 Set takes the Fixtures which you want to add to the Group:
```
Group 1 Attribute 2 Set Fixture 1.1 + 1.2 + 1.3
```
You can also give no Fixture IDs which would result in an empty Group:
```
Group 1 Attribute 2 Set
```
> [!WARNING]
> Please note that Group Attribute 2 Set will first overwrite the Fixtures Attribute.

## Intensities
### Intensity Attribute 2 Set
Intensity Attribute 2 Set sets the Dimmer attribute to the selected Intensities:
```
Intensity 1 + 2 Attribute 2 Set 75
```
Please note that the value is given in percent.

## Colors
### Color Attribute 2 Set
Color Attribute 2 Set sets the Hue attribute of the selected Colors.
The value is given in degree.

### Color Attribute 3 Set
Color Attribute 3 Set sets the Saturation of the selected Colors.
The value is given in percent.

## Cues
### Cue Attribute 2 Set
Cue Attribute 2 Set sets the Intensity of the selected Groups in the selected Cues:
```
Cue 1 + 2 Attribute 2 Group 4 Set Intensity 3
```
You can remove the Intensity like this:
```
Cue 1 + 2 Attribute 2 Group 4 Set -
```

### Cue Attribute 3 Set
Cue Attribute 3 Set sets the Color of the selected Groups in the selected Cues:
```
Cue 1 + 2 Attribute 3 Group 4 Set Color 3
```
You can remove the Color like this:
```
Cue 1 + 2 Attribute 3 Group 4 Set -
```

### Cue Attribute 4 Set
Cue Attribute 4 Set sets the Transition time of the selected Cues.
The value is given in percent.

## Selecting Items
There are some useful tricks for selecting items using IDs.
You can use these whenever you can enter multiple IDs.

### Combining IDs with +
You can combine multiple IDs with +:
```
Intensity 1 + 2.1 + 1.7 Set -
```

### IDs ending with .
This syntax is useful when you want to select all IDs with the same beginning.
However, please note that this command can only select already existing IDs, so you can't use it for Items which don't exist yet!
If they existed, this command would select Intensities 1, 1.1 or any other Intensity with an ID starting with a '1'.
```
Intensity 1. Set -
```

### All items with .
You can simply select all items using a period.
This command, for example, deletes all existing intensities:
```
Intensity . Set -
```

### Selecting multiple items with Thru .
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
