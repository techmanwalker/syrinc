# syrinc's docs

## Architecture

Designed to be modular and multipass, `syrinc`'s flowpath mostly adheres to:

### The command line

Though there *might* be a GUI for this program in the future, right now, the command line interface is the only way to use `syrinc`. Just call the `syrinc` binary for further help and specifics.

The `cli.cpp` file is responsible for basic file i/o and management.

### Metadata/lyrics fetch

For audio files and when the user requires it, the metadata is first extracted from the `LYRICS` metadata tag.
`.lrc` files are just read line by line instead.

### Process lyrics

Due to its modular design, you can freely add or remove processing steps without breaking the output.

By default, it performs:

- **offset correction**: This step reads sequentially the `.lrc` file looking for `[offset:]` tags, and applies its value to all subsequent timestamps found on the lyrics data.
- **metadata drop**: Because `.lrc` files often - and next to all the websites you can download them from **do** - contain data such as the title, the author and the album of the song, this step makes `syrinc` able to drop these redundant tags as they can interfere - and even worse, show up - with what's shown on your favorite music player. This last step is **always** performed when dealing with audio files directly.

### Tokenization/serialization

Lyric processing is token-based, which allows to discriminate between thing like multiple tags, different tag delimiters and also allows it to perform multi-step processing per line.

Doing this e.g. via repeated sub-stringing is inefficient for this case, hence why functions like `tokenize_line` and `serialize_tokens` come with goodies such as `treat_as_lyrics_line` that take some shortcuts in order to produce valid `.lrc` syntax.

This approach means that, no matter how badly an `.lrc` file is formatted, `syrinc` will **automatically format** the syntax for easier human reading and easier program handling.

### Print or save?

The *cli* supports saving directly to a new audio file, overwrite the source audio file via the `-s :in:` option or just print to `stdout` by leaving no output specified. Issue `--help` for more information.


And that's it! For more information, there's also Doxygen documentation integrated in the code itself.