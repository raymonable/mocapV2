# mocapV2 Documentation

mocapV2 runs an HTTP server on port 80. All of the following paths can be accessed with GET requests.

## Tracker Control

### `/trackers`

Returns a list of trackers.<br>
An expected response looks something like:
```json
{
    "trackers": [
        {
            "name": "indexhmd",
            "alias": "head",
            "index": 0
        }
    ]
}
```

### `/trackers/refresh`

Reloads the list of trackers.<br>
Note that the new list of trackers is not returned, you will have to obtain it from `/trackers`.

### `/trackers/live`

Fetches the current position of trackers.<br>
`cframe` is relative to the Roblox CFrame format.

## Mocap

### `/capture/begin`

> :warning: If you use this endpoint, please make sure to use it's counterpart, `/capture/end` at some point, or memory usage will skyrocket!

Starts motion capture.<br>
All trackers will be recorded at 60 frames per second.

### `/capture/end`

Stops motion capture.<br>
If there has been no motion capture data, expect a response like:
```json
{
    "error": "No capture"
}
```
If motion capture was successful, expect a response like:
```json
{
    "count": 1
}
```
Please note the `count` object, which is used to obtain the motion capture data.

### `/capture/fetch?page=n`

Obtains a single block of motion capture data.<br>
This is stored in JSON to make parsing easier.<br>
Each block of motion capture data is the entire JSON string split into ~500 kilobyte chunks.

You are expected to fetch every page (from 1 to the `count` object, obtained from `/capture/end`) and append them together to form a single JSON string.
