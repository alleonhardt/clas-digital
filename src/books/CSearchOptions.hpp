#pragma once

#include <iostream>
#include <string>
#include <vector>


class CSearchOptions
{
private:
    std::string  m_chSearchedWord;              //Searched word
    double m_fuzzyness;                         //0-> no fuzzy search; 
    std::vector<std::string> m_sCollections;    //Vector with all selected pillas
    std::string m_slastName;                    //Search only books from one author
    int m_From;                                 //Search books from year...
    int m_To;                                   //Search books up to yeas...
    bool m_onlyOCR;                             //Search only in ocr 
    bool m_onlyTitle;                           //Search only in the title not in the text

public:

    /**
    * @breif default constructor.
    */
	CSearchOptions();

    /**
    * @brief Constructor
    * @param[in] chSearchedWord searched word
    * @param[in] fuzzyness value of fuzzyness
    * @param[in] sCollections collections in which to be searched
    * @param[in] onlyTitle search only in title?
    * @param[in] onlyOCR search only in ocr (if exists)
    * @param[in] slastName las name of author
    * @param[in] from date from which books shall be searched
    * @param[in] to date to which books shall be searched
    **/
    CSearchOptions(std::string chSearchedWord, int fuzzyness, std::vector<std::string> sCollections,
                                bool onlyTitle, bool onlyOCR, std::string slastName, int from, int to);
    
    /**
    * @brief initialise search options outside of constructor
    * @param[in] chSearchedWord searched word
    * @param[in] fuzzyness value of fuzzyness
    * @param[in] sCollections collections in which to be searched
    * @param[in] onlyTitle search only in title?
    * @param[in] onlyOCR search only in ocr (if exists)
    * @param[in] slastName las name of author
    * @param[in] from date from which books shall be searched
    * @param[in] to date to which books shall be searched
    **/
 	void initialise(std::string chSearchedWord, int fuzzyness, std::vector<std::string> pillar, 
                            bool onlyTitle, bool onlyOCR,std::string slastName, int from, int to);

    //Getter

    /**
    * @return searched word
    **/
    std::string getSearchedWord() const;

    /**
    * @return selected fuzzyness
    **/
    double getFuzzyness() const;

    /**
    * @return selected pillars
    **/
    std::vector<std::string> getCollections() const;

    /**
    * @return whether search only in title 
    **/
    bool getOnlyTitle() const;

    /**
    * @return whether search only in ocr (if exists)
    **/
    bool getOnlyOcr() const;

    /**
    * @return last name of selected author
    */
    std::string getLastName() const;

    /**
    * @return year from which books shall be searched in
    **/
    int getFrom() const;

    /**
    * @return year to which books shall be searched
    **/
    int getTo() const;
};

    

    
