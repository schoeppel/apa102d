# apa102d

## Interesting information as arguments
* LEDs per meter?
* # LEDs
* time since last call

## Lifetime of effects
* step(own_state || NULL, message, timestamp, leds, nr_leds, leds_per_meter) -> own_state
* destroy(own_state)

## Arguments to main
Message with `effect=value0 key1=value1 key2=value2`.

Questions:
* Nested effects? Seems to complex. Also binary message might be easier for arbitrary behavior on embedded devices.
