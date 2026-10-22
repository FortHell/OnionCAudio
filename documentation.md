# Documentation and General Help
Makes the code easier to understand.

## Errors
- Ok = 0
- OpenFailed = 100
- WriteFailed = 101
- ReadFailed = 102
- BadMagic = 103
- BadVersion = 104
- BadHeaderSize = 105
- BadStemCount = 106
- BadChannels = 107
- BadCodec = 108
- BadOutputFormat = 109
- BadVolume = 110
- BadLoop = 111
- BadSampleRate = 112
- ReservedNotZero = 113

## Small Explanations
- Volume: Q4.12, 4 integer bits, 12 fractional bits, between 1 and 10 - essentially just multiplying by 4096