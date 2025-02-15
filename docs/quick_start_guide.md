# Quick Start Guide
## Introduction
Welcome to Zöglfrex, a free software for stage lighting.
In this guide, we will build a simple project in order to show you how to use it.

## The User Interface
On the right side of the Zöglfrex window, you can see the **Inspector**.
This is where we can see all our items.
However, because there are no items added yet, it should be empty.

The left column of the window consists of the **Cuelist View** and the **Terminal**.
In the Cuelist View, we can step through our Cues using the GO and GO BACK buttons.
Here we can also select the Interface we want to use for sending sACN and start or stop the output.

## Setting up the rig
In this tutorial, we want to patch the following lights:
- 12 Dimmers: DMX 1-12
- 8 RGB lamps: DMX 13-36
- 4 Moving Heads (Pan, Tilt, Dimmer, Cyan, Magenta, Yellow): 37-55

### Adding Models
We first need to add the **Models** for our Fixtures.
You can compare this to selecting Fixture Types from a Library.
However, in Zöglfrex they are created quite different.

Every Model can be accessed by an unique **ID**.
We first want to create a Model for our dimmers.
Therefore, we need to type a **command** into the Terminal.
Every command consists of several parts:
- The **Item type**:
    In our case, this will be Model.
- The **selection**:
    In our case, this will be the ID of the Model we want to create.
- The **Attribute**:
    Every Item has different Attributes.
    In our case, we want to set Model Attribute 2 (which are the Channels).
- The **Value**:
    When setting texts, we can't give an Value in the Terminal so a popup box will open.
    But when setting numbers or Item IDs, you have to enter them as a Value.

For adding a new Model, we first need to think of an ID.
We'll take Model ID 1. Now we have to type this command into the Terminal:
```
Model 1 Attribute 2 Set
```
Luckily, you don't have to type every letter into the Terminal, instead, just press the following keys:
1. M
2. 1
3. A
4. 2
5. S

You can find all possible keys and their usage in the Zöglfrex reference.

After inserting the command, you have to execute it by pressung the return key.
When you do this, a popup window will open asking you to insert the Model's channels.
Insert "D" and press the return key again.
The popup should close.

When you have a look into the inspector, you will see that a table appeared there.
It currently contains one Model - the one we've just created.

We now create our second Model, the Generic RGB.
Let's store them at Model ID 2.

However, we can leave the Attribute out because Attribute 2 is the Standard Attribute for Models.
```
Model 2 Set
```
After you've entered this, insert "RGB" into the Channel popup.

No we also have to add a Model for the Moving Heads:
```
Model 3 Set
```
You can enter "00DCMY" in the Channel popup.
After this, we can give Model 3 a proper name:
```
Model 3 Attribute 1 Set
```
Let's call it "Moving Head".

### Adding Fixtures
**Fixtures** represent the physical devices in your rig.

We'll first add our 12 Dimmers.
However, we don't want the IDs of our 12 dimmer Fixtures to be 1 to 12.
Instead, let's take IDs 1.1 to 1.12 which is possible because Zöglfrex allows stacking up multiple numbers behind another.
So we could have also given our Models IDs like 158.518.264.541.123 (but I wouldn't recommend it).
Please note that Zöglfrex ignores .0 at the end of IDs.
This means that we could also select our dimmer Model by typing `Model 1.0.0.0.0` or `Model 1.0.0`.

But now let's patch our dimmers.
In order to do this, it is much faster to select all our new Fixtures and update all of them at once:
```
Fixture 1.1 Thru .12 Attribute 2 Set Model 1
```
This adds our dimmer Fixtures and sets their Model to our dimmer Model.
You could have also typed `Fixture 1.1 + 1.2 + 1.3 + 1.4 + 1.5 + 1.6 + 1.7 + 1.8 + 1.9 + 1.10 + 1.11 + 1.12 Attribute 2 Set Model 1` however, this way the syntax is much shorter.

We still need to patch these Fixtures as they are currently patched to DMX address 0 (which means that they don't generate DMX output):
```
Fixture 1. Set 1
```

We can now also add the RGB fixtures and label them:
```
Fixture 2.1 Thru .8 Attribute 2 Set Model 2
Fixture 2. Set 13
```

And we'll also add the Moving Heads:
```
Fixture 3.1 Thru .4 Attribute 2 Set Model 2
Fixture 3. Set 37
```
In the Inspector, you should now see 12 dimmers, 8 RGBs and 4 Moving Heads.

### Creating Groups
In Zöglfrex, **Groups** are used to control multiple Fixtures at once.
We need to create multiple Groups for our dimmers and RGBs.
For our first Group, we will select all dimmers:
```
Group 1 Set Fixture 1.
```
You can see that the automatically genrated name for our Group is quite complex.
However, we can also give it a custom name:
```
Group 1 Attribute 1 Set
```
This command will open a text input box where you can insert the new Label of these fixtures.
You could call this group "all dimmers".
We now create some more groups for our dimmers:
```
Group 1.1 Set Fixture 1.5 + 1.6 + 1.7 + 1.8
Group 1.1 Attribute 1 Set
Group 1.2 Set Fixture 1.1 + 1.2 + 1.11 + 1.12
Group 1.2 Attribute 1 Set
Group 1.3 Set Fixture 1.3 + 1.4 + 1.9 + 1.10
Group 1.3 Attribute 1 Set
```
Now, we also have to create groups for all of the RGBs.
```
Group 2 Set Fixture 2.
Group 2 Attribute 1 Set
```
...and one for the odd ones...
```
Group 2.1 Set Fixture 2.1 + 2.3 + 2.5 + 2.7
Group 2.1 Attribute 1 Set
```
...and one for the even ones.
```
Group 2.2 Set Fixture 2.2 + 2.4 + 2.5 + 2.8
Group 2.2 Attribute 1 Set
```

And, last but not least, a Group for the Moving Heads:
```
Group 3 Set Fixture 3.
Group 3 Attribute 1 Set
```
Note that as soon as you insert "Model", "Fixture" or "Group" in the Terminal, the Inspector will show a list of all the Models, Fixtures or Groups.
Especially in situations when your setup is very complex, this can help you finding the right ID.

## Adding Values
Now that we have finished setting up our rig, we have to add some values that we can add to our Cue.

### Adding Intensities
An **Intensity** only holds one value, the dimmer which is given in percent:
```
Intensity 1 Set 100
```
Let's create another Intensity:
```
Intensity 2 Set 60
```

### Adding Colors
Now that we have added some Intensities, we also have to add **Colors** for our RGBs:
```
Color 1 Set 0
Color 2 Set 120
Color 3 Set 240
```
The three values represent the Hue of the Colors and are given in degree.
We'll call these Colors "Red", "Green" and "Blue".

### Adding Raws
We can manually set Channels which are not supported (yet) using Raws:
```
Raw 1 Attribute 2 Set 1
Raw 1 Set 157
Raw 2 Attribute 2 Set 2
Raw 2 Set 128
```
You can call these Items "Standard Pan" and "Standard Tilt".

## Adding Cues
### Adding Cues
So let's finally generate some DMX output!
Therefore, we need to add some **Cues** first.
`Cue Set` takes the fade time of the Cue (in seconds) as an parameter:
```
Cue 0 Set 0
Cue 1 Set 5
Cue 1.1 Set 0.5
Cue 2 Set 10
```
After you've done that, you should see that the Cuelist View now shows an Cue ID and a table.
Using the Space bar keys, you can load the next or previous Cue.

Please note that Cues (and all other Items too) are automatically inserted in the right chronological order.
For example, a new Cue with the ID 1.1 would be inserted between Cue 1 and 2.

### Editing Cues
Let's finally add some values to our Cues.
We could accomplish this with this commands:
```
Cue 0 Attribute 2 Group 1.1 Set Intensity 2
Cue 1 Attribute 2 Group 1 Set Intensity 1
Cue 1 Attribute 2 Group 2 Set Intensity 1
Cue 1 Attribute 3 Group 2 Set Color 3
Cue 1.1 Attribute 2 Group 1 Set Intensity 1
Cue 1.1 Attribute 2 Group 2.1 Set Intensity 1
Cue 1.1 Attribute 3 Group 2.1 Set Color 3
Cu1 1.1 Attribute 2 Group 2.2 Set Intensity 1
Cu1 1.1 Attribute 3 Group 2.2 Set Color 2
Cue 2 Attribute 2 Group 1.2 Set Intensity 1
Cue 2 Attribute 2 Group 1.3 Set Intensity 2
Cue 2 Attribute 2 Group 2 Set Intensity 2
Cue 2 Attribute 3 Group 2 Set Color 3
Cue 2 Attribute 2 Group 3 Set Intensity 1
Cue 2 Attribute 3 Group 3 Set Color 2
Cue 2 Attribute 4 Group 3 Set Raw 1 + 2
```
But there's also a much simpler way:
```
Cue 0
Group 1.1
Intensity 2
Cue 1
Group 1
Intensity 1
Group 2
Intensity 1
Color 3
Cue 1.1
Group 1
Intensity 1
Group 2.1
Intensity 1
Color 3
Group 2.2
Intensity 1
Color 2
Cue 2
Group 1.2
Intensity 1
Group 1.3
Intensity 2
Group 2
Intensity 2
Color 3
Group 3
Intensity 1
Color 2
Raw 1 + 2
```

## Activate DMX
In order to activate DMX Output, you have to select your sACN network Interface in the DMX Output Settings.
After that, you're ready to go.

## Final words
This are the most important concepts and commands of Zöglfrex.
For more information, I recommend the Zöglfrex reference.
