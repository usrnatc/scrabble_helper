# Scrabble Helper

Simple program that takes a bunch of letters and displays words able to be
spelt. Options given to; sort the spell-able words lexicographically or by word
length; only display the longest words; and only display words containing a
given letter. A sample scrabble dictionary is provided, but the user can
provide a dictionary if they please.

# Installation
## Linux
```bash
git clone https://github.com/usrnatc/scrabble_helper.git
cd scrabble_helper
make
```

# Usage:
## Linux
```bash
./scrabble_helper [-alpha | -len | -longest] [-i c] letters [dictionary]

    -alpha      ::  displays spell-able words sorted lexicographically
    -len        ::  displays spell-able words sorted by word length
    -longest    ::  displays only the longest spell-able words

    -i c        :: displays only spell-able words containing letter 'c'

    dictionary  :: filepath to custom dictionary file. Format should be words
                   separated by newlines
```

# Other:
Any bugs or errors please let me know :^)
