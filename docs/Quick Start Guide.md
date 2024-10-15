# Quick Start Guide
## Introduction
Welcome to Zöglfrex, a free software for stage lighting.
In this guide, we will build a simple project in order to show you how to use it.

## The User Interface
On the right side of the Zöglfrex window, you can see the **Inspector**.
This is where we can see all our items.
However, because there are no items recorded yet, it should be empty.

The left column of the window consists of the **Cuelist View** and the **Terminal**.
In the Cuelist View, we can step through our Cues using the GO and GO BACK buttons.
Here we can also select the Interface we want to use for sending sACN and start or stop the output.

## Setting up the rig
In this tutorial, we want to patch the following lights:
- 12 Dimmers (DMX 1-12)
- 8 RGB lamps (DMX 13-36)

### Adding Models
We first need to add the **Models** for our Fixtures.
You can compare this to selecting Fixture Types from a Library.
However, in Zöglfrex they are created quite different.

Every Model can be accessed by an unique **ID**.
We first want to create a Model for our dimmers.
Therefore, we need to type a **command** into the Terminal.
Every command consists of several parts:
- The **item type**:
    In our case, this will be Model.
- The **selection**:
    In our case, this will be the ID of the Model we want to record.
- The **operations**:
    Every operation starts with an **operator**.
    In addition to this, some operations also need arguments.
    Currently, there are five operators: Record, Label, Move, Copy and Delete.
    In our case, we will use Record as this operator is used for adding new items.

For adding a new Model, we first need to think of an ID.
We'll take Model ID 1. Now we have to type this command into the Terminal:
```
Model 1 Record
```
Luckily, you don't have to type every letter into the Terminal, instead, just press the following keys:
1. M
2. 1
3. Shift + R

You can find all possible keys and their usage in the Zöglfrex reference.

After inserting the command, you have to execute it by pressung the return key.
When you do this, a popup window will open asking you to insert the Model's channels.
Insert "D" and press the return key again.
The popup should close.

When you have a look into the inspector, you will see that a table appeared there.
It currently contains one Model - the one we've just created.
After this, we want to give them a name so that we know what the IDs stand for.
We do this by executing this command:
```
Model 1 Label
```
Then, we have to type "Dimmer" into the popup window.

We now create our second Model, the Generic RGB.
Let's store them at Model ID 2.

However, we can combine the Record operation and the Label operation in one command:
```
Model 2 Record Label
```
After you've and inserted "RGB" into the Channel popup, you can label them by inserting "RGBs".
In general, it's advisable to label an item right after you've created it.

### Adding Fixtures
**Fixtures** represent the physical devices in your rig.
For adding Fixtures, we use the record command, too.

However, we don't want the IDs of our 12 dimmer Fixtures to be 1 to 12.
Instead, let's take IDs 1.1 to 1.12 which is possible because Zöglfrex allows stacking up to five numbers behind another.
So we could have also given our Models IDs like 158.518.264.541.123 but I wouldn't recommend it.

Please note that if you don't give all ID numbers, Zöglfrex will automatically fill the missing numbers with 0.
This means that we could also select our dimmer Model by typing `Model 1.0.0.0.0` or `Model 1.0.0`.

But now let's patch our dimmers.
In order to do this, it is much faster to select all our new Fixtures and record and label all of them at once:
```
Fixture 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 1.10, 1.11, 1.12 Record 1 Model 1 Label
```
This adds our dimmer Fixtures, sets their Model to our dimmer Model and sets their DMX address to 1 thru 12.
It also sets their label to whatever you entered.
We can now also add the RGB fixtures and label them:
```
Fixture 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8 Record Model 1 Label
```
Please notice that we didn't give an DMX address so Zöglfrex chose the lowest available DMX address possible.
In the Inspector, you should now see 12 dimmers and 8 RGBs.

### Creating Groups
In Zöglfrex, **Groups** are used to control multiple Fixtures at once.
We need to create multiple Groups for our dimmers and RGBs.
For our first Group, we will select all dimmers:
```
Group 1 Record Fixture 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 1.10, 1.11, 1.12 Label
```
You could call this group "all dimmers".
We now create some more groups for our dimmers:
```
Group 1.1 Record Fixture 1.5, 1.6, 1.7, 1.8 Label
Group 1.2 Record Fixture 1.1, 1.2, 1.11, 1.12 Label
Group 1.3 Record Fixture 1.3, 1.4, 1.9, 1.10 Label
```
Now, we also have to create groups for all of the RGBs..
```
Group 2 Record Fixture 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8 Label
```
...and one for the odd ones...
```
Group 2.1 Record Fixture 2.1, 2.3, 2.5, 2.7 Label
```
...and one for the even ones.
```
Group 2.2 Record Fixture 2.2, 2.4, 2.5, 2.8 Label
```
Note that as soon as you insert "Model", "Fixture" or "Group" in the Terminal, the Inspector will show a list of all the Models, Fixtures or Groups.
Especially in situations when your setup is very complex, this can help you finding the right ID.

## Adding Values
Now that we have finished setting up our rig, we have to add some values that we can record to our Cue.

### Adding Intensities
An **Intensity** only holds one value, the dimmer which is given in percent:
```
Intensity 1 Record 100 Label
```
Let's call this Intensity "Full".
Let's create another Intensity and call it "Dimmed":
```
Intensity 2 Record 60 Label
```

### Adding Colors
Now that we have added some Intensities, we also havd to record **Colors** for our RGBs:
```
Color 1 Record 100, 100, 100 Label
Color 2 Record 100, 0, 0 Label
Color 3 Record 0, 100, 0 Label
Color 4 Record 0, 0, 100 Label
```
The three values represent red, green and blue and are also given in percent.
We'll call them "White", "Red", "Green" and "Blue".

### Adding Transitions
A **Transition** defines how to blend to the current Cue.
Every Transition holds one value, the fade time given in seconds.
We'll record some Transitions too:
```
Transition 1 Record 5 Label
Transition 2 Record 0 Label
Transition 3 Record 0.5 Label
Transition 4 Record 10 Label
```
Let's call them "Standard", "Cut", "Fast" and "Slow".
Please notice that we entered a value of half a second for Transition 3 using the period key.
You can also record decimal values for Intensities or Colors the same way.

## Recording Cues
### Adding Cues
So let's finally generate some DMX output!
Therefore, we need to add some **Cues** first:
```
Cue 0 Record Transition 2
Cue 1 Record Transition 1
Cue 1.1 Record Transition 3
Cue 2 Record Transition 4
```
After you've done that, you should see that the Cuelist View now shows an ID and a table.
Using the "GO and "GO BACK" keys, you can load the next or previous Cue.

Please note that Cues (and all other Items too) are automatically inserted in the right chronological order.
For example, a new Cue with the ID 1.1 would be inserted between Cue 1 and 2.

### Adding Rows
The rows in the Cuelist View table are called **Rows**.
When creating a Row, you have to assign a Group to it:
```
Row 1 Record Group 1 Label
Row 1.1 Record Group 1.1 Label
Row 1.2 Record Group 1.2 Label
Row 1.3 Record Group 1.3 Label
Row 2 Record Group 2 Label
Row 2.1 Record Group 2.1 Label
Row 2.2 Record Group 2.2 Label
```
In the case that multiple Rows contain the same fixture, the order of the row decides about the generated output.
The higher Row ID will take precedence.

### Editing Cues
Let's finally add some values to our Cues:
```
Cue 0 Record Row 1.1 Intensity 2
Cue 1 Record Row 1 Intensity 1 Record Row 2 Intensity 1 Color 4
Cue 1.1 Record Row 1 Intensity 1 Record Row 2.1 Intensity 1 Color 4 Record Row 2.2 Intensity 1 Color 2
Cue 2 Record Row 1.2 Intensity 1 Record Row 1.3 Intensity 2 Record Row 2 Intensity 2 Color 3
```

### Activating sACN output
However, you still need to activate sACN output.
Therefore, you need to select a network interface and press "Start sACN Output".
Zöglfrex now starts generating DMX data for the current Cue and transmitting it via sACN.

## Final words
This are the most important concepts and commands of Zöglfrex.
For more information, I recommend the Zöglfrex reference.