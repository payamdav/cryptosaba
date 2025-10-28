## Snapshot Analyze ##
* This is a study of analyze and predicting crypto movement.
* The focus is at analyze. but at the end this module tries to predict the movement
* scenario starts at choosing a point in time ( by secound ts), it can be a random or specific point within a valid time range that we have enough data for that. from a defined symbol.
* for each point the module loads 3 ranges of 1s candles
    - last 1 week: to calculate norms, behavier, averages and overal characteristics of symbol
    - last 1 day: it is analytical range that can be used to create long time and short time context, trends, price zones, barriers and so on
    - next 4 hours: used to validate prediction

* as at the time of development the data is available from 20250310 to 20250404 the valid range could be 20251017 to 20250403 and symbols are btcusdt, ethusdt, adausdt, xrpusdt, vineusdt, trumpusdt, dogeusdt
* an executable in scripts/studies/snapshot_analyze.cpp will instantiate class and runs analyze method. setting ts as random with fixed seed and one of symbols.
* the overal structure of analyzer is a class with name of Analyze that its constructor gets a symbol and a timestamp. there also must be 4 vectors of Candle:
    - l7d
    - l1d
    - n1d
    - l1dn1d: it is 2 days vector that starts from last day ends to next day also used for prediction validity and label making
* also some more data must be calculated:
    - volume_normalized: normalized volume of 1d (devided by average volume 1 week)
    - average candle size ( based on 1 week)
    - last 1 day prices offseted from current price and scaled by average candle size
    - all prices are based on vwap price
    - current price is vwap of current candle. but considered as 0 because it offsetted by itself
* for visualization the data must exported to bin or csv files (I prefer binary) after that another project in js will visualize for me
* also I need l1dn1d versions of volume normalized and ofsetted scaled price for visualizing and maybe further calculation. 
* export l1dn1d prices and vols and also symbol and current ts to files. also export l1dn1d original candles.

## Creating label for snapshot
Label here means the one that we may use to feed in machine learnong models. so the label may tell us how the next swing of price will goes. it is better  for that label to be a double number. 
* at first step I prefer to have a smooth curve of weighted price for next day. so It is better to have lowess of vwap weighted by v on whole l1dn1d
