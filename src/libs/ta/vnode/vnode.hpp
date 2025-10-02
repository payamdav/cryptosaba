#pragma once

// #include "../../statistics/live_stats.hpp"


// Vnode class subscribes to candles that provided to it as candle pointer. 
// it calculates average and std of volumes based on period that provided to it. and considers candles that has average + x * std of volume as hv (high volume)
// another way also it uses kernel convolution to smooth the volume data before calculating statistics. ( type of volume and kernel size are configurable ) (kernel applies on the volume data. kernel applies on previous candles within the period)
// hv candles is start of hv area and consecutive hv candles form a high volume area. during hv area a flag is set to true and upon exiting the area, the flag is reset and area finished message published to pubsub.
