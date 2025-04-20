/*
 * stock.h - fetch stock data
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2025-04-20
 *
 * https://curl.se/libcurl/c/simple.html
 *
 * https://github.com/json-c/json-c
 *
 * https://stackoverflow.com/questions/61976027/scraping-yahoo-finance-at-regular-intervals
 */

#ifndef STOCK_H
#define STOCK_H

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

/*
 * Stock value struct
 */
typedef struct stock_value_t
{
  int    time;
  int    volume;
  double high;
  double low;
  double close;
  double open;
} stock_value_t;

/*
 * Stock struct
 */
typedef struct stock_t
{
  char*          symbol;
  char*          name;
  char*          exchange;
  char*          range;
  char*          interval;
  char*          currency;
  int            volume; // Regular Market Volume

  int            start;  // Today Start Time
  int            end;    // Today End   Time
  double         open;   // Today Open  Price
  double         close;  // Today Close Price
  double         high;   // Today High  Price
  double         low;    // Today Low   Price

  stock_value_t* values;
  size_t         value_count;

  stock_value_t* _values;
  size_t         _value_count;
  int            _start;
  int            _end;
  double         _open;
  double         _close;
  double         _high;
  double         _low;
} stock_t;

/*
 * Function declarations
 */

extern stock_t* stock_create(char* symbol);

extern int      stock_zoom(stock_t* stock, char* range);

extern int      stock_resize(stock_t* stock, size_t count);

extern int      stock_update(stock_t* stock);

extern void     stock_free(stock_t** stock);

#endif // STOCK_H

#ifdef STOCK_IMPLEMENT

#include <curl/curl.h>
#include <json-c/json.h>

/*
 * Stock ranges and corresponding intervals
 */

const char* STOCK_RANGES[]    = { "1d", "1wk", "1mo", "1y", "max" };

const char* STOCK_INTERVALS[] = { "1m", "15m", "30m", "1h", "1d" };

#define STOCK_RANGE_COUNT    (sizeof(STOCK_RANGES)    / sizeof(char*))

#define STOCK_INTERVAL_COUNT (sizeof(STOCK_INTERVALS) / sizeof(char*))

/*
 * Get index of stock range string
 */
static inline ssize_t stock_range_index_get(const char* range)
{
  for (ssize_t index = 0; index < STOCK_RANGE_COUNT; index++)
  {
    if (strcmp(STOCK_RANGES[index], range) == 0)
    {
      return index;
    }
  }

  return -1;
}

/*
 * Get stock interval string of index
 */
static inline const char* stock_interval_get(size_t index)
{
  if (index >= 0 && index < STOCK_INTERVAL_COUNT)
  {
    return STOCK_INTERVALS[index];
  }

  return NULL;
}

/*
 * Get interval that corresponds to range
 */
static inline const char* stock_range_interval_get(const char* range)
{
  ssize_t index = stock_range_index_get(range);

  if (index == -1)
  {
    return NULL;
  }
  
  return stock_interval_get(index);
}

/*
 * Calculate stock start, end, open, close, high and low for 1 day
 */
static inline int stock_meta_calc(stock_t* stock)
{
  if (stock->value_count <= 0)
  {
    return 1;
  }

  stock_value_t value = stock->values[stock->value_count - 1];

  stock->end   = value.time;
  stock->close = value.close;

  stock->high = value.high;
  stock->low  = value.low;

  for (size_t index = stock->value_count; index-- > 0;)
  {
    value = stock->values[index];

    stock->high = MAX(stock->high, value.high);
    stock->low  = MIN(stock->low,  value.low);

    stock->start = value.time;
    stock->open  = value.open;
  }

  return 0;
}

/*
 * Calculate stock _start, _end, _open, _close, _high and _low values
 */
static inline int stock_values_calc(stock_t* stock)
{
  if (stock->_value_count <= 0)
  {
    return 1;
  }

  stock_value_t value = stock->_values[stock->_value_count - 1];

  stock->_end   = value.time;
  stock->_close = value.close;

  stock->_high = value.high;
  stock->_low  = value.low;

  for (size_t index = stock->_value_count; index-- > 0;)
  {
    value = stock->_values[index];

    stock->_high = MAX(stock->_high, value.high);
    stock->_low  = MIN(stock->_low,  value.low);

    stock->_start = value.time;
    stock->_open  = value.open;
  }

  return 0;
}

/*
 * Resize stock values and store them in _values
 */
int stock_resize(stock_t* stock, size_t count)
{
  if (count > stock->value_count)
  {
    return 1;
  }

  size_t group_size = stock->value_count / count;

  size_t spill = stock->value_count - count * group_size;

  stock_value_t* values = malloc(sizeof(stock_value_t) * count);

  if (!values)
  {
    return 2;
  }

  size_t value_index = 0;
  
  for (size_t group_index = 0; group_index < count; group_index++)
  {
    // Get first value of group
    stock_value_t group_value = stock->values[value_index++];

    size_t curr_size = (group_index < spill) ? group_size + 1 : group_size;

    // Merge middle values of group
    for (size_t index = 1; index < (curr_size - 1); index++)
    {
      stock_value_t value = stock->values[value_index++];

      group_value.high = MAX(group_value.high, value.high);
      group_value.low  = MIN(group_value.low,  value.low);
    }
  
    // Merge last value of group
    if (curr_size > 1)
    {
      stock_value_t value = stock->values[value_index++];

      group_value.close  = value.close;
      group_value.time   = value.time;
      group_value.volume = value.volume;

      group_value.high = MAX(group_value.high, value.high);
      group_value.low  = MIN(group_value.low,  value.low);
    }

    values[group_index] = group_value;
  }

  free(stock->_values);

  stock->_values = values;

  stock->_value_count = count;

  if (stock_values_calc(stock) != 0)
  {
    // Maybe use copy stock and then return error here
  }

  return 0;
}

/*
 * Function for curl to write response
 */
static inline size_t stock_response_write(void* ptr, size_t size, size_t nmemb, char* response)
{
  size_t total_size = size * nmemb;

  strncat(response, ptr, total_size);

  return total_size;
}

#define STOCK_URL_SIZE 256
#define STOCK_URL_BASE "https://query1.finance.yahoo.com/v8/finance/chart/"

/*
 * Create url for fetching stock data
 */
static inline char* stock_url_create(char* symbol, char* range, char* interval)
{
  if (!symbol)
  {
    return NULL;
  }

  char* url = malloc(sizeof(char) * STOCK_URL_SIZE);

  if (!url)
  {
    return NULL;
  }

  if (sprintf(url,"%s%s?", STOCK_URL_BASE, symbol) < 0)
  {
    free(url);

    return NULL;
  }

  if (range && sprintf(url + strlen(url), "range=%s&", range) < 0)
  {
    free(url);

    return NULL;
  }

  if (interval && sprintf(url + strlen(url), "interval=%s&", interval) < 0)
  {
    free(url);

    return NULL;
  }

  return url;
}

#define STOCK_RESPONSE_SIZE 1000000

#define STOCK_CURL_HEADER "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3"

/*
 * Get curl response for a stock
 */
static inline char* stock_response_get(char* symbol, char* range, char* interval)
{
  curl_global_init(CURL_GLOBAL_DEFAULT);

  CURL* curl = curl_easy_init();

  if (!curl)
  {
    curl_global_cleanup();

    return NULL;
  }

  char* response = malloc(sizeof(char) * STOCK_RESPONSE_SIZE);

  if (!response)
  {
    curl_global_cleanup();

    return NULL;
  }

  memset(response, '\0', sizeof(char) * STOCK_RESPONSE_SIZE);

  char* url = stock_url_create(symbol, range, interval);

  if (!url)
  {
    free(response);

    curl_global_cleanup();

    return NULL;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);

  curl_easy_setopt(curl, CURLOPT_USERAGENT, STOCK_CURL_HEADER);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, stock_response_write);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

  CURLcode res = curl_easy_perform(curl);


  curl_easy_cleanup(curl);

  curl_global_cleanup();

  free(url);

  if (res == CURLE_OK)
  {
    return response;
  }

  fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

  free(response);

  return NULL;
}

/*
 * Parse stock name, either longName or shortName, or symbol
 */
static inline int stock_name_parse(stock_t* stock, struct json_object* meta)
{
  struct json_object* name = json_object_object_get(meta, "longName");

  if (name && json_object_is_type(name, json_type_string))
  {
    stock->name = strdup(json_object_get_string(name));

    return 0;
  }

  error_print("Missing 'longName' field: %s", stock->symbol);


  name = json_object_object_get(meta, "shortName");

  if (name && json_object_is_type(name, json_type_string))
  {
    stock->name = strdup(json_object_get_string(name));

    return 0;
  }

  error_print("Missing 'shortName' field: %s", stock->symbol);


  stock->name = strdup(stock->symbol);

  return 1;
}

/*
 * Parse stock meta data
 */
static inline int stock_meta_parse(stock_t* stock, struct json_object* result)
{
  struct json_object* meta = json_object_object_get(result, "meta");

  if (!meta)
  {
    error_print("Missing 'meta' field: %s", stock->symbol);

    return 1;
  }


  struct json_object* currency = json_object_object_get(meta, "currency");

  if (!currency || !json_object_is_type(currency, json_type_string))
  {
    error_print("Missing 'currency' field: %s", stock->symbol);

    return 3;
  }

  stock->currency = strdup(json_object_get_string(currency));


  if (stock_name_parse(stock, meta) != 0)
  {
    error_print("Failed to parse stock name");
  }


  struct json_object* exchange = json_object_object_get(meta, "fullExchangeName");

  if (!exchange || !json_object_is_type(exchange, json_type_string))
  {
    error_print("Missing 'fullExchangeName' field: %s", stock->symbol);
  }

  stock->exchange = strdup(json_object_get_string(exchange));


  struct json_object* volume = json_object_object_get(meta, "regularMarketVolume");

  if (!volume || !json_object_is_type(volume, json_type_int))
  {
    error_print("Missing 'regularMarketVolume' field: %s", stock->symbol);
  }

  stock->volume = json_object_get_int(volume);

  return 0;
}

/*
 * Parse json objects for stock value
 */
static inline int stock_value_parse(stock_value_t* value, struct json_object* time, struct json_object* volume, struct json_object* open, struct json_object* close, struct json_object* high, struct json_object* low)
{
  if (!time || !json_object_is_type(time, json_type_int))
  {
    return 1;
  }

  value->time = json_object_get_int(time);


  if (!volume || !json_object_is_type(volume, json_type_int))
  {
    return 2;
  }

  value->volume = json_object_get_int(volume);


  if (!open || !json_object_is_type(open, json_type_double))
  {
    return 3;
  }

  value->open = json_object_get_double(open);


  if (!close || !json_object_is_type(close, json_type_double))
  {
    return 4;
  }

  value->close = json_object_get_double(close);


  if (!high || !json_object_is_type(high, json_type_double))
  {
    return 5;
  }

  value->high = json_object_get_double(high);


  if (!low || !json_object_is_type(low, json_type_double))
  {
    return 6;
  }

  value->low = json_object_get_double(low);

  return 0;
}

/*
 * Parse stock values
 */
static inline int stock_values_parse(stock_t* stock, struct json_object* result)
{
  struct json_object* indicators = json_object_object_get(result, "indicators");

  if (!indicators)
  {
    error_print("Missing 'indicators' field: %s", stock->symbol);

    return 1;
  }


  struct json_object* quote = json_object_object_get(indicators, "quote");

  if (!quote || !json_object_is_type(quote, json_type_array))
  {
    error_print("Missing 'quote' field: %s", stock->symbol);

    return 2;
  }

  quote = json_object_array_get_idx(quote, 0);


  struct json_object* time = json_object_object_get(result, "timestamp");

  if (!time || !json_object_is_type(time, json_type_array))
  {
    error_print("Missing 'timestamp' field: %s", stock->symbol);

    return 3;
  }


  struct json_object* volume = json_object_object_get(quote, "volume");

  if (!volume || !json_object_is_type(volume, json_type_array))
  {
    error_print("Missing quote 'volume' field: %s", stock->symbol);

    return 4;
  }


  struct json_object* open = json_object_object_get(quote, "open");

  if (!open || !json_object_is_type(open, json_type_array))
  {
    error_print("Missing quote 'open' field: %s", stock->symbol);

    return 5;
  }


  struct json_object* close = json_object_object_get(quote, "close");

  if (!close || !json_object_is_type(close, json_type_array))
  {
    error_print("Missing quote 'close' field: %s", stock->symbol);

    return 6;
  }


  struct json_object* high = json_object_object_get(quote, "high");

  if (!high || !json_object_is_type(high, json_type_array))
  {
    error_print("Missing quote 'high' field: %s", stock->symbol);

    return 7;
  }


  struct json_object* low = json_object_object_get(quote, "low");

  if (!low || !json_object_is_type(low, json_type_array))
  {
    error_print("Missing quote 'low' field: %s", stock->symbol);

    return 8;
  }

  size_t count = json_object_array_length(open);

  stock->values = malloc(sizeof(stock_value_t) * count);

  if (!stock->values)
  {
    error_print("Failed to malloc stock values");

    return 9;
  }

  stock->value_count = 0;

  for (size_t index = 0; index < count; index++)
  {
    stock_value_t* value = &stock->values[stock->value_count];

    if (stock_value_parse(value,
      json_object_array_get_idx(time, index),
      json_object_array_get_idx(volume, index),
      json_object_array_get_idx(open, index),
      json_object_array_get_idx(close, index),
      json_object_array_get_idx(high, index),
      json_object_array_get_idx(low, index)
    ) == 0)
    {
      stock->value_count++;
    }
  }

  return 0;
}

/*
 * Get stock data from the internet
 */
static inline int stock_fetch(stock_t* stock)
{
  char* response = stock_response_get(stock->symbol, stock->range, stock->interval);

  if (!response)
  {
    return 1;
  }

  struct json_object* json = json_tokener_parse(response);
  
  free(response);

  if (!json)
  {
    error_print("json_tokener_parse");

    return 2;
  }

  struct json_object* chart = json_object_object_get(json, "chart");

  if (!chart)
  {
    error_print("Missing 'chart' field: %s", stock->symbol);

    json_object_put(json);

    return 3;
  }

  struct json_object* result = json_object_object_get(chart, "result");

  if (!result || !json_object_is_type(result, json_type_array))
  {
    error_print("Missing 'result' field: %s", stock->symbol);

    json_object_put(json);

    return 4;
  }

  result = json_object_array_get_idx(result, 0);

  if (stock_meta_parse(stock, result) != 0)
  {
    json_object_put(json);

    return 5;
  }

  if (stock_values_parse(stock, result) != 0)
  {
    json_object_put(json);

    return 6;
  }

  json_object_put(json);

  stock_resize(stock, stock->value_count);

  return 0;
}

/*
 * Free data of stock
 */
static inline void stock_data_free(stock_t* stock)
{
  free(stock->values);

  free(stock->_values);

  free(stock->symbol);

  free(stock->name);

  free(stock->exchange);

  free(stock->currency);

  free(stock->range);

  free(stock->interval);
}

/*
 * Free stock object
 */
void stock_free(stock_t** stock)
{
  if (!stock || !(*stock)) return;

  stock_data_free(*stock);

  free(*stock);

  *stock = NULL;
}

/*
 * Zoom existing stock to specified range and update 1d meta data
 *
 * On error, stock is not affected
 */
int stock_zoom(stock_t* stock, char* range)
{
  const char* interval = stock_range_interval_get(range);

  if (!interval)
  {
    return 1;
  }

  stock_t copy = (stock_t)
  {
    .symbol   = strdup(stock->symbol),
    .range    = strdup(range),
    .interval = strdup(interval),
  };

  if (stock_update(&copy) != 0)
  {
    stock_data_free(&copy);

    return 2;
  }

  stock_data_free(stock);

  *stock = copy;

  return 0;
}

/*
 * Update stock by fetching both specified range and 1d meta data
 */
int stock_update(stock_t* stock)
{
  // 1. Fetch and update 1d meta data
  char* range = "1d";

  const char* interval = stock_range_interval_get(range);

  if (!interval)
  {
    return 1;
  }

  stock_t day = (stock_t)
  {
    .symbol   = strdup(stock->symbol),
    .range    = strdup(range),
    .interval = strdup(interval),
  };

  if (stock_fetch(&day) != 0)
  {
    stock_data_free(&day);

    return 1;
  }

  if (stock_meta_calc(&day) != 0)
  {
    stock_data_free(&day);

    return 2;
  }

  // 2. Fetch and update range values
  stock_t copy = (stock_t)
  {
    .symbol   = strdup(stock->symbol),
    .range    = strdup(stock->range),
    .interval = strdup(stock->interval),
  };

  if (stock_fetch(&copy))
  {
    stock_data_free(&day);

    stock_data_free(&copy);

    return 3;
  }

  // Perserve 1d meta data
  copy.high  = day.high;
  copy.low   = day.low;
  copy.open  = day.open;
  copy.close = day.close;
  copy.start = day.start;
  copy.end   = day.end;

  stock_data_free(&day);

  stock_data_free(stock);

  *stock = copy;

  return 0;
}

/*
 * Create stock with symbol and 1d range data
 */
stock_t* stock_create(char* symbol)
{
  char* range = "1d";

  const char* interval = stock_range_interval_get(range);

  if (!interval)
  {
    return NULL;
  }

  stock_t* stock = malloc(sizeof(stock_t));

  if (!stock)
  {
    return NULL;
  }

  memset(stock, 0, sizeof(stock_t));

  *stock = (stock_t)
  {
    .symbol   = strdup(symbol),
    .range    = strdup(range),
    .interval = strdup(interval),
  };

  if (stock_fetch(stock) != 0)
  {
    stock_free(&stock);

    return NULL;
  }

  if (stock_meta_calc(stock) != 0)
  {
    stock_free(&stock);

    return NULL;
  }

  return stock;
}

#endif // STOCK_IMPLEMENT
