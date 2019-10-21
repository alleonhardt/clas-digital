class CSearchOptions:

    def __init__(self, word, fuzzyness, onlyTitle, onlyOcr, author, dateFrom, dateTo, collections):
        self.word = word
        self.fuzzyness = fuzzyness
        self.onlyTitle = onlyTitle
        self.onlyOcr = onlyOcr

        self.author = author
        self.dFrom = dateFrom
        self.dTo   = dateTo

        self.collections = collections


