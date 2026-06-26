# Quick Start Guide
## Introduction
Welcome to Zöglfrex, a free software for stage lighting.
In this guide, we will build a simple project in order to show you how to use it.

## Creating a Project File
First of all, you need to create a Project File.
Therefore, you need to run the Zöglfrex AppImage, click the *New File* button and select the location of the file.

> [!CAUTION]
> Please note that Zöglfrex files always have to end with .zfr

If the given file already exists, Zöglfrex will open it.
Otherwise, Zöglfrex will create an empty file with this name.

## The User Interface
On the right side of the Zöglfrex window, you can see the **Inspector**.
This is where we can see all our items.
However, because there are no items added yet, it should be empty.

The left column of the window consists of the **Cuelist View** and the **Terminal**.
In the Cuelist View, we can see our current Cue or Group, select Highlight or see the progress of the fade between two Cues.
In the Terminal, we can insert commands and see their results.

## Setting up the rig
In this tutorial, we want to patch the following lights:
- 12 dimmers: DMX 1-12
- 8 RGB lamps: DMX 13-36
- 4 moving heads (pan, tilt, dimmer, gobo, cyan, magenta, yellow): 37-58

### Adding Models
We first need to add the **Models** for our Fixtures.
You can compare this to selecting Fixture Types from a Library.
However, in Zöglfrex they are created quite different.

Every Model can be accessed by an unique **ID**.
We first want to create a Model for our dimmers.
Therefore, we need to type a command into the Terminal.
Every command consists of several parts:
- The **Item type**:
    In our case, this will be Model.
- The **selection**:
    In our case, this will be the ID of the Model we want to create, in our case 1.
- The **Attribute**:
    Every Item has different Attributes.
    In our case, we want to set Model Attribute 2 (which are the Channels).
- The **Value**:
    When setting texts, we can't give an Value in the Terminal so a popup box will open.
    But when setting numbers or Item IDs, you have to enter them as a Value.

For adding a new Model, we first need to think of an ID.
We'll take Model ID 1. Now we have to type this command into the Terminal:
```
Model 1 Set +
```
Luckily, you don't have to type every letter into the Terminal, instead, just press the following keys:
1. M
2. 1
3. S
4. +

You can find all possible keys and their usage in the [Zöglfrex reference](reference.md).

After inserting the command, you have to execute it by pressing the return key.
This will create our new Model.

When you have a look into the Inspector, you will see that a table appeared there.
It currently contains one Model - the one we've just created.

Now, we need to specifiy the channels offered by the Model.
Therefore, we run the following command:

```
Model 1 Attribute 2 Set
```
When you execute this command, a popup window will open asking you to insert the Model's channels.
Insert "D" and press the return key again.
The popup should close.

We now create our second Model, the Generic RGB.
Let's store them at Model ID 2.

```
Model 2 Set +
Model 2 Attribute 2 Set
```
After you've entered this, insert "RGB" into the Channel popup.

No we also have to add a Model for the moving heads:
```
Model 3 Set +
Model 3 Attribute 2 Set
```
You can enter "PTD0CMY" in the Channel popup.

After this, we can give our Models a proper name:
```
Model 1 Set
Model 2 Set
Model 3 Set
```
We'll call Model 1 "Dimmer", Model 2 "RGB" and Model 3 "Moving Head".

### Adding Fixtures
**Fixtures** represent the physical devices in your rig.

We'll first add our 12 dimmers.
However, we don't want the IDs of our 12 dimmer Fixtures to be 1 to 12.
Instead, let's take IDs 1.1 to 1.12 which is possible because Zöglfrex allows stacking up multiple numbers behind another.
So we could have also given our Models IDs like 158.518.264.541.123 (but I wouldn't recommend it).
Please note that Zöglfrex ignores .0 at the end of IDs.
This means that we could also select our dimmer Model by typing `Model 1.0.0.0.0` or `Model 1.0.0`.

But now let's patch our dimmers.
In order to do so, it is much faster to select all our new Fixtures at once:
```
Fixture 1.1 Thru .12 Set +
```
This adds our 12 dimmer Fixtures.
You could have also typed `Fixture 1.1 + 1.2 + 1.3 + 1.4 + 1.5 + 1.6 + 1.7 + 1.8 + 1.9 + 1.10 + 1.11 + 1.12 Set +` however, this way the syntax is much shorter.

For setting the Model of these Fixtures to our dimmer Model, we need to execute this:
```
Fixture 1. Attribute 2 Set Model 1
```
Here, we used an even shorter syntax for selecting our Fixtures.

We still need to patch these Fixtures as they are currently patched to DMX address 0 (which means that they don't generate DMX output):
```
Fixture 1. Set 1 Thru 12
```

We can now also add the RGB fixtures and label them:
```
Fixture 2.1 Thru .8 Set +
Fixture 2. Set Model 2
Fixture 2. Set 13 Thru 34
```

And we'll also add the moving heads:
```
Fixture 3.1 Thru .4 Set +
Fixture 3. Set Model 2
Fixture 3. Set 37 Thru 58
```
In the Inspector, you should now see 24 Fixtures: 12 dimmers, 8 RGBs and 4 moving heads.

### Creating Groups
In Zöglfrex, **Groups** are used to control multiple Fixtures at once.
We need to create multiple Groups for our dimmers, RGBs and moving heads.
For our first Group, we will select all dimmers:
```
Group 1 Set +
Group 1 Set Fixture 1.
```
We can also give it a custom name:
```
Group 1 Set
```
This command will open a text input box where you can insert the new label of this Group.
You could call this group "all Dimmers".
We now create some more groups for our dimmers:
```
Group 1.1 Thru .3 Set +
Group 1.1 Set Fixture 1.5 + 1.6 + 1.7 + 1.8
Group 1.1 Set
Group 1.2 Set Fixture 1.1 + 1.2 + 1.11 + 1.12
Group 1.2 Set
Group 1.3 Set Fixture 1.3 + 1.4 + 1.9 + 1.10
Group 1.3 Set
```
Now, we also have to create groups for all of the RGBs...
```
Group 2 Set +
Group 2 Set Fixture 2.
Group 2 Set
```
...and one for the odd ones...
```
Group 2.1 Set +
Group 2.1 Set Fixture 2.1 + 2.3 + 2.5 + 2.7
Group 2.1 Set
```
...and one for the even ones.
```
Group 2.2 Set +
Group 2.2 Set Fixture 2.2 + 2.4 + 2.5 + 2.8
Group 2.2 Set
```

And, last but not least, a Group for the moving heads:
```
Group 3 Set +
Group 3 Set Fixture 3.
Group 3 Set
```
Note that as soon as you insert "Model", "Fixture" or "Group" in the Terminal, the Inspector will show a list of all the Models, Fixtures or Groups.
Especially in situations when your setup is more complex, this can help you finding the right ID.

## Adding Values
Now that we have finished setting up our rig, we have to add some values that we can add to our Cues.

### Adding Intensities
An **Intensity** only holds one value, the Dimmer, which is given in percent:
```
Intensity 1 Set +
Intensity 1 Set 100
```
Let's create another Intensity:
```
Intensity 2 Set +
Intensity 2 Set 60
```

### Adding Colors
Now that we have added some Intensities, we also have to add **Colors** for our RGBs:
```
Color 1 + 2 + 3 Set +
Color 1 Set 0
Color 2 Set 120
Color 3 Set 240
```
The three values represent the Hue of the Colors and are given in degree.

### Adding Positions
For controlling our Moving Heads, we will also need some **Positions**:
```
Position 1 Set +
Position 1 Attribute 3 Set - 45
```
We'll call this Position "Audience".

### Adding Raws
We can manually set channels which are not supported (yet) using **Raws**, just like the Gobo channel of our Moving Heads:
```
Raw 1 Set +
Raw 1 Attribute 2.4 Model 3 Set 157
```
You can call this Raw "Gobo".
When you set this Raw for a moving head Fixture, it will set Channel 4 (the Gobo channel) to the DMX value 157.

## Adding Cues
### Adding Cues
So let's finally generate some DMX output!
Therefore, we need to add a **Cuelist** first:
```
Cuelist 1 Set +
Cuelist 1 Set
```
We will call this Cuelist "Main Playback".

Now, we can add **Cues** to the Cuelist:
```
Cue 0 + 1 + 1.1 + 2 Set +
```

We can set the Fade time of the Cues (in seconds) like this:
```
Cue 0 Set 0
Cue 1 Set 5
Cue 1.1 Set 0.5
Cue 2 Set 10
```

After you've done that, you should see that the Cuelist View now shows an Cue ID and a table.
Using the space key, you can load the next Cue.
By holding the shift key and pressing the space key, you can move back to the previous Cue again. 

Please note that Cues (and all other Items too) are automatically inserted in the right chronological order.
For example, a new Cue with the ID 1.1 would be inserted between Cue 1 and 2.

### Editing Cues
Let's finally add some values to our Cues:
```
Cue 0
Group 1.1
Intensity 2
Cue 1
Intensity -
Group 1
Intensity 1
Group 2
Intensity 1
Color 3
Cue 1.1
Intensity -
Color -
Group 2.1
Intensity 1
Color 3
Group 2.2
Intensity 1
Color 2
Cue 2
Group 1
Intensity -
Group 1.2
Intensity 1
Group 1.3
Intensity 2
Group 2.1
Intensity -
Color -
Group 2.2
Intensity -
Color -
Group 2
Intensity 2
Color 3
Group 3
Intensity 1
Color 2
Position 1
Raw 1
```

## Activate DMX
In order to activate DMX output, you have to select your sACN network interface in the *DMX Output Settings*.
After that, you're ready to go.

## Final words
This are the most important concepts and commands of Zöglfrex.
For more information, I recommend the [Zöglfrex reference](reference.md).
