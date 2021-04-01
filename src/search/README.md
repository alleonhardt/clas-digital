# Fast C++ Word-based (fuzzy) search.

When writing this search we dealt with two problems: 

1. very shitty text- base,

2. very huge text-base (~20000 different meta-data, ~1GB of text)

We wanted to come as close as possible to constant search time for normal- and <1sec for fuzzy search. 
## Tasks
What we want to be able to do:
- normal-/ fuzzy-search in text-base and meta-data, sorting the results by
  relevance and presenting the user with a short preview of the found word.
- normal-/ fuzzy-search in a single book, showing all pages on which the book
  was found, and what word was found on this page.
- fuzzy-search should find matches _containing_ the searched word and _similar_
  words. (which is especially needed because of the shitty 
- search for n words near each other (on same page)

## What is needed
We decided to make the search word-base, meaning, that we only compare the searched word, with single words.
To achieve the specified tasks, we need:

- a list of __all words__
- a knowledge of __the books where the word occurs__
- Potentially a __relevance__ stored, to sort the search-results by 
- The (or at least one) __position__ on which the word was found, to quickly generate a preview.
- __all pages__ on which the word was found to show the user where the word was
  found, and (for searching multiple words) 


## Data-structures.
For this we developed two Data-structures: 
1. Map of _all words_: mapping a word to a list of books and every book to a relevance and a scope (metadata (1), corpus (2), both (3)).

```c++
BookManager:: std::unordered_map<string, map<string, MatchObject>> map_of_words_;
```

With `MatchObject` being:
```c++
struct MatchObject {
  double relevance;
  _int8 scope // 1: only metadata, 2: scope, 3: metadata+scope.
}

```
2. Map of all words in _a book_: mapping every base-form to a list of conjunctions. Every conjunction has a `WordInfo` containing
   further information (pages, relevance, first-preview-position)
```c++
Book:: std::unordered_map<string, std::vector<WordInfo>> map_of_words_pages_;
```

With `WordInfo` being:
```c++
struct WordInfo {
  std::string word_;
  std::vector<size_t> pages_;
  size_t preview_position_;
  size_t preview_page_;
  double relevance_;
}
```

Assuming however, we found a word with fuzzy-search. F.e. we searched for
"Phifosophie" (typo on input), and found "Philosophie". How do we assure, that
this word can later be found, to get the preview, or the pages?
For each books, we store all words, not existing in the `Book::map_of_words_pages_` in the following two maps: 

```c++
std::unordered_map<string, SortedMatches> metadata_fuzzy_matches;
std::unordered_map<string, SortedMatches> corpus_fuzzy_matches;
```

With `SortedMatches` being:
```c++
class SortedMatches {
  private:
    typedef std::pair<std::string, double> weighted_match;
    typedef std::function<bool(const weighted_match&, const weighted_match&)> Cmp;
    std::set<weighted_match, Cmp> matches_;
  public:
    void Insert(weighted_match new_match); // inserts, resorts (O(10)), keep size<10
    std::string GetBestMatch();
    std::vector<std::string> GetAllMatches();
}
```

The first string (the key) is the searched word, which does not exist in
`CBook::map_of_words_`.
The `SortedMatches` object represents a sorted set of matches, that _do_ exist in the map of 
words. We keep these sorted, so, when retrieving matches, we can be sure, to always find the best matches.
The matches are kept below 10 to keep the insert-operation as "cheep" as
possible.


Also a few things should be guaranteed:
- every word is in __lower-case__
- for every word __all non-utf-8-charactes are replaced__ by similar characters: `'ä'->'a', ...`
- if existing, then every word should be converted to it's __base-form__: `"hunden"->"hund" ...` 

To find a word, we need to apply the same conversion to the query as to the stored words. Also we may want to store the query into as separate words (`"Armeise+Krieg"-> ["Armeise", "Krieg"`)
This makes it necessary to have a data-structure for the search, which will be created for both `full_search` and `seach_in_book`.

```c++
class SearchObject {
  const std::string query_;
  std::vector<string> words_;	// created in constructor
  std::vector<string> converted_words_;	// created in constructor
  SearchOptions search_options_;
}
```

With `SearchOptions` being:

```c++
class SearchOptions {
    bool fuzzy_search_;
    bool only_metadata_;
    bool only_corpus_;
    int from_;	// year
    int to_;	// year
    int sort_result_; // 0=relevance, 1=chronologically, 2=alphabetically
    std::string author_; // last-name of author.
    std::vector<std::string> collections_; 
}
```

Also we need an object storing the results. During the search the result-objects will be stored in a map `std::map<std::string, ResultObject>` to easy remove duplicates (the key being the book-key). After the search is done, the map will be sorted and converted to a list: `std::list<ResultObject`.  

```c++
class ResultObject {
  bool found_in_metadata_;
  bool found_in_corpus_;
  int meta_data_score_;
  int corpus_score;
  Book* book;
}
```

It is obvious, that this object is only used, for the full search (not "search in book", i.e. `Book::get_pages()`. 
The stored (scope), makes it easy to use this object, when f.e. createing the preview. 
As searching for multiple words it might be useful to store the cope for each
word. Also it might be useful to stick to the same scope design used in
`MatchObject`. This would result in the `ResultObject` looking as follows:

```c++
class ResultObject {
  Book* book_;
  std::vector<MatchObject> 
}
```

## Algorithms

### Creating map of pages:

__NOTE__: _up to the step_ `ConvertToBaseFormMap()` _we are using a temporary
structure, which will later be converted to the_ `Book::map_of_words_`
structure. The temporary structure looks as follows:
```c++
Book:: std::unordered_map<string, WordInfo> map_of_words_;
class TempWordInfo {
  typedef std::pair<size_t, size_t> weighted_match; ///< page:relevance.
  typedef std::function<bool(const weighted_match&, const weighted_match&)> Cmp;
    
  //Membervariable
  std::set<weighted_match, Cmp> pages_with_relevance_; ///< set for sorting + dublicates.
  size_t preview_position_;  ///< Position best preview.
  size_t preview_page_;  ///< Page on which preview is found.
  int relevance_;  ///< Relevance (how often word occures on pages).
}
```
The class has several functions making the creation of the index map easier.
Also it stores the pages sorted by relevance. The relevance of the page is given
by the number of actual words. This gives us the possibility to create better
preview. (IN a former step we simply chose the first page).

#### CreatePages()
Splits pages at specified page-break. Calls CreatePage();

Summary:
- split pages, call `CreatePage(cur_page, page_number)` for each page.

#### CreatePage(Current Page (string), Page Number (unsigned int))
Extracts all words from current page (ExtractWordsFromString).
Each word is then added to the `CBook::map_of_words_`, the current page-number
is added to the pages and the relevance is updated.

`ExtractWordsFromString` also modifies the current page (it adds spaces, where
they are missing), so we save the page to the disc, after adding all words. This
will later result in having each page saved separately, to make loading previews
easier. 

Summary:
- extract all words from page and add to map (calls `ExtractWordsFromString(current_page)`).
- update pages and relevance in `WordInfo`.
- safe modified page to disc as single page.

#### ExtractWordsFromString(Current Page (string))
This is the central functions for splitting up a page into separate words. It
also does more than that. It tries to identify, strings, which are no words, and
it removes strange characters from the strings. (__Note__: _All of this is done
with more are the less primitive methods. This is a place for potential
progress_).

Summary: 
- add a space after every `'.', ',', ';', ':'` ("... ging unter.Ich..." ->
  "...ging unter. Ich...") (this is the only change for the buffer)
- Split current page at " ".
- iterate over all resulting strings
  - calls `transform()` which erases all trims word from all leading and trailing non-alpha-characters. (__NOTE__: transform does does not erase non-utf-8 characters (such as "ä", "Ö", etc.)
  - skips if string is to long or to short ([2, 25]) or `is_word()` is false.
  - adds word to result-map and increases relevance if already existed.
- returns a page-rank = number of no-skipped words.

__NOTE__: _at this point the neither have the non-utf-8-characters been
replaced, nore has the word been converted to lower-case. All of this will be
done after finding the preview_.

#### AddPreviews()
For every word in the map a preview is created. We will create the preview for
the top-ranked page (= page with the most words). 

Summary:
- calls `FindBestPreview()` for every word.

#### FindBestPreview(Current Word (string))

Summary: 
- gets page with the highest number of words.
- finds and returns the position of the given word `100000` otherwise. (we will definitely find the word, as no conversion has taken place)

#### ConvertToBaseFormMap()

Summary:
Convert words:
- As we now found the best preview position, we can convert the string:
  We do this by calling `ConvertWords` which
  1. Converts each word in the current index-map:
    - convert string to lower-case.
    - call `convertStr()`, replacing non-utf-8 characters, by utf-8-characters
  2. For every resulting duplicate (maybe "Hund" and "hund" existed, now we
     need to handle that we have to seperate `TempWordInfo`s for "hund") we
     "join" the WordInfos:
     - Find best preview position (preview on page with higher relevance)
     - Set preview-position and preview-page accordingly.
     - Join all pages: Page_A ∪ Page_B (no need to keep 
     - TODO: Handle pages_with_relevance_ this would need joining too.
Create Baseforms:
- iterates over all words in current map of pages.
- for every word: 
    - search base-form,
    - `base_form = (GetBaseForm(cur_word) == "") ? cur_word : base_form;` So the
      base_form is now either the base-form, or (if not found exists) the current word itself.
    - create (real) word-info for current word __Note that the current word is__
      __used rather than the baseform__.
    - `map_words_pages_[base_form].push_back(word_info);` Either creating a new
      entry (if the current word/ baseform is not tracked) and always adding the
      current word as a entry for this base-form.
  - As now we have the number of pages, we can construct the relevance as
    `relevance/ number_of_pages` 

### Find Preview
First of: with out a lot of work and close to O(1) every preview should be found easily, as
the word for which a preview should be found either exists in the map of words
(so we have the page and position of the preview), or it exists either in the
`metadata_fuzzy_matches`-map, or the `corpus_fuzzy_matches`-map and we simply
find our entry in the map of words by taking the first (and best, as corresponding 
sets are sorted) entry in one of these maps.

What are our goals for a preview:
- no longer than 150 characters.
- the found word should be marked 

#### GetPreview(n words (original/ converted), fuzziness, scope)
Note: the scope for all words is the same: either "meta-data" or "corpus", the
scope is determined by where the word was found.
Summary:
- call `FindPreview(first word, fuzziness, scope)` 
- for all n+1 words:
  - check if word could be found in first preview. 
  - if yes: call `HighLightMatchInPreview(text=first_preview, pos)`.
  - if no: call `FindPreview(nth word, fuzziness, scope)` 

#### FindPreview(word (original/ converted, fuzziness, scope)
Summary:
- call `FindPreviewText(word (converted/ original), fuzziness)` or `FindPreviewMetadata(word (converted/ original), fuzziness)` both return a position and a string the not-shortened preview.
- call `TrimString(not-shortened, pos)` trimming the string to 150 characters.
- call `HighLightMatchInPreview(text, pos)` which highlights the searched word.
- return preview.

#### FindPreviewText(word (converted/ original), fuzziness)
Summary:
- find word in map_of_words.
- if not found and `fuzziness==true`:
  - find word in `corpus_fuzzy_matches` and take the first (and automatically best)
    corresponding set entry. 
- (__NOTE__: _if we still haven't found our word, then something went
    horribly wrong and we're fucked, as our algos are corrupted!_)
- As this is only the base-form, do a quick fuzzy matching with the original
  searched word and find the best match under base-form and all others.
- set preview-position to the given preview-position of the entry in `WordInfo` 
- load page from disc from corresponding `WordInfo` preview-position-page and
  return.

#### FindPreviewMetadata(word (converted/ original), fuzziness)
Summary:
- find word in title.
- if not found and `fuzziness==true`:
  - find word in `metadata_fuzzy_matches` and take the first (and automatically best)
    corresponding set entry. 
- (__NOTE__: _if we still haven't found our word it might be the date or author
  of the book, and we're not as fucked as in the above case._)

### Search
- Create `SearchObject` 
- fuzziness false:
  - Search for first word in corpus (simple look-up)
    - for every found word generate `ResultObject` and set `found_in_corpus_=true`
    also set `corpus_score`.
  - search for first word in metadata (simple look-up)
    - for every found word generate `ResultObject` and set `found_in_corpus_=true`
      also set `corpus_score`.
  - if n==1: sort, convert to list of `ResultObjects` and return.
  - for n+1 words: set first results as result-list, then execute above steps and for every word in result-list:
    - if word not new-result list: remove word from result list.
    - if word _and_ corresponding word in result list where only found in corpus:
      - if `book->OnSamePage(word1, word2)` == false: remove word from result-list.
  - sort, convert to list of `ResultObjects` and return.
- fuzziness true: 
  - Search for first word in corpus (done by iterating + levensthein)
    - for every found word generate `ResultObject` and set `found_in_corpus_=true`
    also set `corpus_score`.
  - search for first word in metadata (done by iterating + levensthein)
    - for every found word generate `ResultObject` and set `found_in_corpus_=true`
      also set `corpus_score`.
  - if n==1: sort, convert to list and return.
  - for n+1 words set first results as result-list:
    - repeat above steps, however, don't apply fuzzy search if not in
      result-list.
  - if word _and_ corresponding word in result list where only found in corpus:
      - if `book->OnSamePage(word1, word2)` == false: remove word from result-list.
  - sort, convert to list of `ResultObjects` and return.

### Search in book.
- Create `SearchObject` (with fuzziness as only search-option set)
- find word in map_of_words.
- if not found and `fuzziness==true`:
  - find word in `corpus_fuzzy_matches` and take the first (and automatically best)
    corresponding set entry. 
- (__NOTE__: _if we still haven't found our word, then something went
    horribly wrong and we're fucked, as our algos are corrupted!_)
- IN contrast to the preview-case, we want to find as many pages as possible, so
  we iterate over all conjunctions of our base-form and for each page we add the
  page = current word to the result-list.
- if n>1: find some funny algo, which makes sure that all matches not in all
  result-list are removed.

## Metadata 
What has not been discussed a lot is how to search for the metadata. In general,
there are two different approaches:
1. Decide on a basic set of tags which should be searched. (f.e. authors, title,
   year, date)
2. Allow searching all tags.

As storing metadata is not very costy and also the map-based-design reduces the
stored data additionally, we will go for the second approach.

### Storing metadata 
The metadata will basically be stored just like any other data, also storing
conjunctions of a baseform with certain information to later find the
preview of the searched word.

```c++
Book:: std::unordered_map<string, std::vector<WordInfo>> map_of_metadata_;
```

We can use the same `WordInfo` structure to achieve this, the words only need a
slight different meaning:
- `word_` is obviously simply the word.
- `pages_` can be seen as `locations` with each location being an `location-id`. 
- `preview_position_` stays the same. 
- `preview_page_` becomes `preview_location` and determines on which location
  the preview can be found. 
- `relevance_` stays the same. 

The hard point is the location-id. The first assumption would be to say the
location-id is the json-path to the specific tag. However this would consume way
to much RAM. So we expect a config saying which tags to use for searching. The
we create a new json, where all the needed tags are stored, under a certain id.
Conceder the following json: 
```json
{
  "data": {
      "ISBN": "978-3-412-50582-0 978-3-412-50658-2",
      "bookTitle": "Tiere: Begleiter des Menschen in der Literatur des Mittelalters",
      "collections": [
          "XCFFDRQC"
      ],
      "creators": [
          {
              "creatorType": "editor",
              "firstName": "Andreas",
              "lastName": "Kra\u00df"
          },
          {
              "creatorType": "editor",
              "firstName": "Judith",
              "lastName": "Klinger"
          },
          {
              "creatorType": "author",
              "firstName": "Lina",
              "lastName": "Herz"
          }
      ],
      "date": "2017",
      "dateAdded": "2020-08-20T22:13:48Z",
      "dateModified": "2020-08-20T22:19:03Z",
      "itemType": "bookSection",
      "key": "KX8EW9D7",
      "language": "ger",
      "libraryCatalog": "Gemeinsamer Bibliotheksverbund ISBN",
      "place": "K\u00f6ln Weimar Wien",
      "shortTitle": "Der Hund",
      "title": "Der Hund"
  }
}
```

Assuming the user wanted to search for alls authors (as authors), all editors (as editors), 
title or bookTitle (as title, prefering bookTitle) and  shortTitle, we would expect the following 
json as a config. 

```json
{
  "searchableTags":{
      "title": { 
        "tags_or":["data/bookTitle", "data/title"],
        "relevance":2
      },
      "shortTitle": {
        "tag":"data/shortTitle",
        "relevance":1
      },
      "authorsFirstNames": {
        "tag":"data/creators?creatorType=author/firstName",
        "relevance":2
      },
      "authorsLastNames": {
        "tags_or": ["data/creators?creatorType=author/lastName", "data/creators?creatorType=author/name"],
        "relevance":2
      },
      "editorsFirstNames": {
        "tag":"data/creators?creatorType=editor/firstName",
        "relevance":2
      },
      "editorsLastNames": {
        "tag":"data/creators?creatorType=editor/lastName",
        "relevance":2
      }
      "key": {
        "tag":"key",
        "relevance":0
      }
      "collections": {
        "tag":"data/collections",
        relevance:0
      }
  },
  "representations": {
      "authors": {"join": ["authorsLastNames", "authorsFirstNames"], "separator":", "}
      "editors": {"join": ["editorsLastNames", "editorsFirstNames"], "separator":", "}
  }
}
```

We also included the "key", which is needed to store the books. In general there
will later be a set of values, which _must be included_ (key, author, title),
however theres might also be left bankl.
The field "representations" allows the user to build string from certains
values, f.e. join the first and last names of authors or editors. Thus the
following json would be the result:


```json
{
  "title": { 
    "value":"Tiere: Begleiter des Menschen in der Literatur des Mittelalters", 
    "relevance":2
  },
  "shortTitle": {
    "value":"Der Hund",
    "relevance":1
  },
  "authors": { 
    "value":"Herz, Lina",
    "relevance", 2
  }
  "editors": {
    "Klinger, Judith, Kraüf, Andreas",
    "relevance":2,
  }
  "key": {
    "value":"KX8EW9D7",
    "relevance":0
  }
}
```

Important rules for the relevance are: 
- `relevance = 0` → won't be searched (in this example, we are given the path to
  the key, however it won't be searched).
- the relevance of a representations will be the average of all relevance used
  to build the representations (thus `authorsFirstNames` and `-LastNames` both
  had relevance `2`)

Important riles for necessary tags:
- "key" a tag is needed, to clearly identify one item.
- "collections" as it is expected that each book is part of a certain collection. 
If however no collections are used for your search case, collections might as
well be an empty string (still the tag is needed).
- "description" if sorting alphabetically shall be supported natively (of course
  it is possible to sort alphabetically with the search-resulsts anyway)
- "date" if sorting chronologically shall be supported and if one expects to use
  the search-options (in particularly the search-options frome-date and
  to-date)_


## Calculating relevance
The relevance is calculated closly to the okapi index, with some sligh
difference:

### Term frequency.
The term frequency `ft(t,d)` for a term in a document is calculated as `f(t,d)/|(d, c|m)|`
with `f(t,d)_ =  occurences of word in document` and `|d|= number of different
words in metadata or corpus.` (Depending on, where the word occures (metadata/
corpus).

As we store all words just as the base-form, when creating the index map in
`BookManager::CreateIndexMap` or more specific: `BookManager::AddWordsFromItem`
the term frequency is calculated as the combined term_frequency of all words
subsumised under this base-form. 
