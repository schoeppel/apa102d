# apa102d

## Interesting information as arguments
* LEDs per meter?
* # LEDs
* time since last call? -- timestamp is better

## Lifecycle of effects
### void* step(void* last_state || NULL, char** message, unsigned long long timestamp, struct apa102_led* leds, int nr_leds, int leds_per_meter)
`step` is called every time a new frame is needed. The first time it is called, the `last_state` is `NULL`.

### destroy(void* own_state)
When closing down, `destory` is called with the runtime state of the effect, returned by `step`.

## Arguments to main
Message with `effect=value0 key1=value1 key2=value2`.

Questions:
* Nested effects? Seems to complex. Also binary message might be easier for arbitrary behavior on embedded devices.
