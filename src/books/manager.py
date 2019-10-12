import book

class CManager:
    def __init__ (self):
        self.mapBooks = {}

    def updateZotero(self, j_items):
        for it in j_items: 
            myBook = book.CBook(it)
            self.mapBooks[it["key"]] = myBook
