class CBook:
    def __init__(self, metadata):
        self.metadata = metadata
        self.key=metadata["key"]
    def getAuthor(self):
        try: 
            return self.metadata["data"]["creators"][0]["lastName"] 
        except:
            try:
                return self.metadata["data"]["creators"][0]["name"] 
            except:
                return "no author"

        

   
