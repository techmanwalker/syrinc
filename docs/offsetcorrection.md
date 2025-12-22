
# syrinc

## A more technically detailed explanation on how offset correction works

Between the audio players that support showing you lyrics as the music plays, no matter if it is done by saving standalone `.lrc` files or directly embedding lyrics into the audio file metadata, they don't all agree on how to *actually* show up the lyrics.

For example, you have this line:

```
[offset: 500]
[00:10.00] Don't say I didn't, say I didn't warn you
```

You might think:

> *10 seconds plus 500 milliseconds means delaying the line by 500 ms, right?*

Actually, that *depends* on whatever the player thinks it's a good idea to do. Some decide to delay as you'd expect, but turns out most players out there actually **advance** 500 ms, so a positive offset actually *advances* time rather than *delay* the timestamp.

This tool is designed to give you absolute fine control on how the offset of the lyrics actually behaves. By running what I internally called *offset correction*, there's no way the player mistakes on how to handle timestamps or offsets anymore.

### Example

Suppose you have this `lyrics.lrc` file:

```
[offset: 750]
[00:23.24] Every time that I look in the mirror
```

Some players might interpret that 750 ms offset as *delay* or *advance* by 750 ms. But by running this command:

```
syrinc -f lyrics.lrc -s :in:
```

`syrinc` will parse your `lyrics.lrc` file, read the `[offset]` tag, and will apply that offset value over all the timestamps that **follow next** to that tag.
Hence, your `lyrics.lrc` becomes:

```
[00:22.49] Every time that I look in the mirror
```

Do you notice how the output timestamp is lower than the source timestamp? `syrinc` applied that 750 ms advance offset over the timestamp and now your `lyrics.lrc` file is curated to work exactly the same way with all the players.