# syrinc

**Music lyrics metadata editor**

---

`syrinc` is an audio file handler which lets you embed, format and convert lyrics data to your favorite songs, to make them display and move along in your music player.

## Example usages

```
# Embed lyrics from an .lrc file in a song
syrinc -f audio.flac -l lyrics.lrc -s :in:

# Export lyrics from song metadata to an external .lrc file
syrinc -f audio.flac -s lyrics.txt

# Hardcode the lyrics offset so lyrics show at the right time on all players
syrinc -f audio.flac -s :in:
```

## On finding and using `.lrc` files

### Where to download `.lrc` files for my songs

If your player supports showing lyrics, you can always download `.lrc` files for your songs from [Lyricsify](https://www.lyricsify.com/) or [Megalobiz](https://www.megalobiz.com/) (this last one may look weird but it actually holds `.lrc` files for a lot of worldwide-available songs), use `syrinc` to embed them in your audio files and getting them displayed.

### What players support showing lyrics for my songs?

*Feel free to suggest more players on the pinned issue.*

- [Elisa](https://apps.kde.org/es/elisa/), by KDE *(Windows, Linux)*
- [Poweramp](https://play.google.com/store/apps/details?id=com.maxmpz.audioplayer), by MaxMP *(Android)*

## Build instructions

For any Linux:

```
# Build syrinc
cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Install syrinc
sudo cmake --install build

# Uninstall syrinc
sudo cmake --build build --target uninstall
```
