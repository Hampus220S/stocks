/*
 * stock.h - fetch stock data
 *
 * Written by Hampus Fridholm
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
  char*          range;
  char*          interval;
  char*          currency;
  stock_value_t* values;
  size_t         value_count;
  double         high;
  double         low;
} stock_t;

/*
 * Function declarations
 */

extern stock_t* stock_get(char* symbol, char* range, char* interval);

extern void     stock_free(stock_t** stock);

#endif // STOCK_H

#ifdef STOCK_IMPLEMENT

#include <curl/curl.h>
#include <json-c/json.h>

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
 * Parse stock meta data
 */
static inline int stock_meta_parse(stock_t* stock, struct json_object* result)
{
  struct json_object* meta = json_object_object_get(result, "meta");

  if (!meta)
  {
    error_print("Missing 'meta' field");

    return 1;
  }

  struct json_object* symbol = json_object_object_get(meta, "symbol");

  if (!symbol || !json_object_is_type(symbol, json_type_string))
  {
    error_print("Missing 'symbol' field");

    return 2;
  }

  stock->symbol = strdup(json_object_get_string(symbol));


  struct json_object* currency = json_object_object_get(meta, "currency");

  if (!currency || !json_object_is_type(currency, json_type_string))
  {
    error_print("Missing 'currency' field");

    return 3;
  }

  stock->currency = strdup(json_object_get_string(currency));


  struct json_object* name = json_object_object_get(meta, "longName");

  if (!name || !json_object_is_type(name, json_type_string))
  {
    error_print("Missing 'longName' field");

    return 4;
  }

  stock->name = strdup(json_object_get_string(name));


  struct json_object* range = json_object_object_get(meta, "range");

  if (!range || !json_object_is_type(range, json_type_string))
  {
    error_print("Missing 'range' field");

    return 5;
  }

  stock->range = strdup(json_object_get_string(range));

  return 0;
}

/*
 * Parse json objects for stock value
 */
static inline int stock_value_parse(stock_value_t* value, struct json_object* timestamp, struct json_object* open, struct json_object* close, struct json_object* high, struct json_object* low)
{
  if (!timestamp || !json_object_is_type(timestamp, json_type_int))
  {
    return 1;
  }

  value->time = json_object_get_int(timestamp);


  if (!open || !json_object_is_type(open, json_type_double))
  {
    return 2;
  }

  value->open = json_object_get_double(open);


  if (!close || !json_object_is_type(close, json_type_double))
  {
    return 3;
  }

  value->close = json_object_get_double(close);


  if (!high || !json_object_is_type(high, json_type_double))
  {
    return 4;
  }

  value->high = json_object_get_double(high);


  if (!low || !json_object_is_type(low, json_type_double))
  {
    return 5;
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
    error_print("Missing 'indicators' field");

    return 1;
  }


  struct json_object* quote = json_object_object_get(indicators, "quote");

  if (!quote || !json_object_is_type(quote, json_type_array))
  {
    error_print("Missing 'quote' field");

    return 2;
  }

  quote = json_object_array_get_idx(quote, 0);


  struct json_object* timestamp = json_object_object_get(result, "timestamp");

  if (!timestamp || !json_object_is_type(timestamp, json_type_array))
  {
    error_print("Missing 'timestamp' field");

    return 3;
  }


  struct json_object* open = json_object_object_get(quote, "open");

  if (!open || !json_object_is_type(open, json_type_array))
  {
    error_print("Missing 'open' field");

    return 4;
  }


  struct json_object* close = json_object_object_get(quote, "close");

  if (!close || !json_object_is_type(close, json_type_array))
  {
    error_print("Missing 'close' field");

    return 5;
  }


  struct json_object* high = json_object_object_get(quote, "high");

  if (!high || !json_object_is_type(high, json_type_array))
  {
    error_print("Missing 'high' field");

    return 6;
  }


  struct json_object* low = json_object_object_get(quote, "low");

  if (!low || !json_object_is_type(low, json_type_array))
  {
    error_print("Missing 'low' field");

    return 7;
  }

  size_t count = json_object_array_length(open);

  stock->values = malloc(sizeof(stock_value_t) * count);

  if (!stock->values)
  {
    error_print("Failed to malloc stock values");

    return 8;
  }

  for (size_t index = 0; index < count; index++)
  {
    stock_value_t* value = &stock->values[stock->value_count];

    if (stock_value_parse(value,
      json_object_array_get_idx(timestamp, index),
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
stock_t* stock_get(char* symbol, char* range, char* interval)
{
  char* response = stock_response_get(symbol, range, interval);

  if (!response)
  {
    return NULL;
  }

  stock_t* stock = malloc(sizeof(stock_t));

  if (!stock)
  {
    free(response);

    return NULL;
  }

  memset(stock, 0, sizeof(stock_t));

  struct json_object* json = json_tokener_parse(response);
  
  free(response);

  if (!json)
  {
    free(stock);

    error_print("json_tokener_parse");

    return NULL;
  }

  struct json_object* chart = json_object_object_get(json, "chart");

  if (!chart)
  {
    free(stock);

    error_print("Missing 'chart' field");

    json_object_put(json);

    return NULL;
  }

  struct json_object* result = json_object_object_get(chart, "result");

  if (!result || !json_object_is_type(result, json_type_array))
  {
    free(stock);

    error_print("Missing 'result' field");

    json_object_put(json);

    return NULL;
  }

  result = json_object_array_get_idx(result, 0);

  if (stock_meta_parse(stock, result) != 0)
  {
    free(stock);

    json_object_put(json);

    return NULL;
  }

  if (stock_values_parse(stock, result) != 0)
  {
    free(stock);

    json_object_put(json);

    return NULL;
  }

  json_object_put(json);

  if (stock->value_count > 0)
  {
    stock_value_t value = stock->values[0];

    stock->high = value.high;
    stock->low  = value.low;

    for (size_t index = 1; index < stock->value_count; index++)
    {
      value = stock->values[index];

      stock->high = MAX(stock->high, value.high);
      stock->low  = MIN(stock->low,  value.low);
    }
  }

  return stock;
}

/*
 * Free stock struct
 */
void stock_free(stock_t** stock)
{
  if (!stock || !(*stock)) return;

  free((*stock)->values);

  free((*stock)->symbol);

  free((*stock)->name);

  free((*stock)->currency);

  free((*stock)->range);

  free((*stock)->interval);

  free(*stock);

  *stock = NULL;
}

#endif // STOCK_IMPLEMENT
