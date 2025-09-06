# cpp-search-server
Final project: search server

A demo project of a search server that searches through document texts with the ability to specify stop words (ignored by the server), minus words (documents with them are not taken into account in the results).

The program searches for documents containing query keywords, shows the results in an ordered manner - starting with the most useful ones. Usefulness is determined by the relevance of the document. The relevance of a document to a query is determined using TF (term frequency - the share that a given word occupies among all the words in the document) and IDF (inverse document frequency - the more documents contain a word, the lower the IDF).

## How it works
Creating an instance of the SearchServer class. A string with stop words separated by spaces is passed to the constructor. Instead of a string, you can pass an arbitrary container (with sequential access to elements with the ability to use in a for-range loop)

Documents for search are added using the AddDocument method. The method is passed the document id, status, rating, and the document itself in string format.

The FindTopDocuments method returns a vector of documents according to the passed keywords. The results are sorted by the TF-IDF statistical measure. Additional filtering of documents by id, status, and rating is possible.

The RequestQueue class implements a queue of requests to the search server with saving of search results.

## Deployment and requirements
C++17. No additional requirements.
