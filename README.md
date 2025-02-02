# Scrabble Helper

This tool helps find spellable words out of some given letters.

The tool currently uses dictionary.txt to search however a dictionary file can be provided.
**NOTE: in dictionary file, words must be separated by newlines**

The tool is multithreaded, currently just taking the number of CPU cores.
No way of specifying number of threads to use at this point in time.

```
Example: ./sch "aeuild" -i f -s -d "./dictionary.txt" -r

Spellable word selection:
    -r                         allow characters within jumbled_letters to repeatedly be used
    -i c                       all found words must include letter 'c'
    -d dictionary_file_path    use wordlist found in dictionary_file_path
                               NOTE: words need to be line separated and lowercase

Output control:
    -s    sort found spellable words by word size
    -a    sort found spellable words lexicographically

Miscellaneous:
    -h    display this help message
```