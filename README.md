# Wordle resolver

Find a word with a good heuristic to discover the word of the day


## Example with word ROBIN

```
< oater
```
:yellow_square: :black_large_square: :black_large_square: :black_large_square: :green_square: 
```
> ybbby
< louis
```
:black_large_square: :green_square: :black_large_square: :green_square: :black_large_square: 
```
> bgbgb
< amnic
```
:black_large_square: :black_large_square: :yellow_square: :green_square: :black_large_square:
```
> bbygb
< robin
```
:green_square: :green_square: :green_square:: :green_square: :green_square:

## Emscripten

You can use the script online https://ender-events.github.io/wordle-resolver/ (Firefox only)

## Terminal

```bash
git clone git@github.com:Ender-events/wordle-resolver.git
cd wordle-resolver
make resolver
./resolver
```
