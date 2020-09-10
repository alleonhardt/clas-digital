let search_parameters = {};
let default_search_parameters = {};


let jsonObj = {}
let lastInput ="";
let filterOpts = {};
let gCurrentSearchId = -1;

function OpenFilter(x) {
	let retval = x.value;
	let lnk = window.location.search;
	if(getParameterByName('start')!=null) {
	  lnk = window.location.search.replace("&start=" + getParameterByName('start'),"");
	}

	let val = getParameterByName(x.id);
	if (val==null) {
	  window.location = lnk + "&"+x.id+"="+retval;
  }
	else {
    if (x.value == "10" || x.value == "relevance")
      window.location = lnk.replace("&"+x.id+"="+val,"");
    else
      window.location = lnk.replace(x.id+"="+val,x.id+"="+retval);
	}
}

function ResetFocus() {
  document.getElementById("SpecialSID").focus();
} 

function SetAdvancedYear(x) {
  if(x.value==1) {
    document.getElementById("CustomDateOpt").style.display = "block";
    document.getElementById("publicatedAfter").focus();
  }
  else if(x.value==0) {
    document.getElementById("publicatedAfter").value=1700;
    document.getElementById("publicatedBefore").value=2049;
    document.getElementById("SpecialSID").focus();
  }
  else {
    let after = parseInt(x.value);
    document.getElementById("publicatedAfter").value=after;
    document.getElementById("publicatedBefore").value=after+99;
    document.getElementById("SpecialSID").focus();
  }
}

function SetCustom() {
  let after = document.getElementById("publicatedAfter").value;
  let before = document.getElementById("publicatedBefore").value;
  let year = document.getElementById("year");
  year.remove(6);
  let opt = document.createElement("option");
  opt.value=2;
  opt.text=after + " till " + before;
  year.add(opt);
  year.value=2;

  document.getElementById("CustomDateOpt").style.display = "none";
  document.getElementById("SpecialSID").focus();
}

function changeColor(x) {
  x.style.color="#8B008B";
}

function callMetadata(x) {
  window.location = "/books/"+x.dataset.scanid+"/";
}

function ShowSelectedValues(obj) {
  
  // *** ADD RESULTS FOOTER *** //
  json = obj.drawablejson;
  document.getElementById("maxResults").innerHTML = json.max_results;
  document.getElementById("searchTime").innerHTML = json.time;
  document.getElementById("ResultBanner").style.display = "block";

	if (json.max_results==0)
	  return;

  document.getElementById("selAll").style.display = "block";

  //Set start and limit and calculate the number of books being presented.
  let start = 0;
  if(getParameterByName('start')!=null)
    start = parseInt(getParameterByName('start'));
  let limit = 10;
  if(getParameterByName("limit") != null)
    limit = parseInt(getParameterByName("limit"));
  let to=limit;
  if(start+limit > json.max_results)
    to = json.max_results - start;

  //Iterate over books and add to page.
	for (var i=0; i<to; i++) {
    let newList = document.createElement("li");
    newList.id = json.books[i].scanId;

    let div1 = document.createElement("div")
    div1.setAttribute("class", "searchResult");

    let desc = document.createElement("span");
    if(!json.books[i].hasocr || (json.books[i].hasocr == undefined))
      desc.innerHTML = json.books[i].description;

    else {
      desc.innerHTML = "<a onclick='changeColor(this);return true;' href='/books/"
        + json.books[i].scanId
        + "/pages/?highlight="+document.getElementById("SpecialSID").value;
        if (document.getElementById("fuzzyness").value != 0) {
          desc.innerHTML+="&fuzzyness="+document.getElementById("fuzzyness").value+"'>";
        desc.innerHTML += json.books[i].description+"</a>";
    }
    desc.setAttribute("class", "desc");
    div1.appendChild(desc);

    let bib = document.createElement("span");
    bib.innerHTML = "<a class='metadata' href='/books/" + json.books[i].scanId
                    + "/'>" + json.books[i].bibliography + "</a>";
    div1.appendChild(bib);

    let prev = document.createElement("span");
    prev.innerHTML = json.books[i].preview;
    prev.setAttribute("class", "preview");
    div1.appendChild(prev);
    newList.appendChild(div1);

    let div2 = document.createElement("div");
    div2.setAttribute("class", "tickbox");
    let aBox = document.createElement("input");
    aBox.setAttribute("class", "booklistinp");
    aBox.setAttribute("type", "checkbox");
    aBox.setAttribute("data-scanid", json.books[i].scanId);
    div2.appendChild(aBox);
    newList.appendChild(div2);
    hitList.appendChild(newList);
  }

  document.getElementById("bottomBorder").style.display="block";

  let lastHit = json.max_results - ((json.max_results-start) % limit);
  let numPages = (json.max_results - (json.max_results % limit))/limit;
  let counter = numPages+1;
  if(start%limit != 0) counter++;

  let linkcontainer = document.getElementById("linkcontainer");
  for(let i = lastHit; i >= 0; i-=limit) {
    let newlst = document.createElement("li");
    newlst.setAttribute("class", "link");

    let newlnk='';
    if(getParameterByName('start')==null)
        newlnk = window.location.search+"&start="+i;
    else
        newlnk = window.location.search.replace("start="+start,"start="+i);
    if(i==start)
        newlst.innerHTML = counter;
    else
        newlst.innerHTML = "<a href='"+newlnk+"'>"+counter+"</a>";
    linkcontainer.insertBefore(newlst, linkcontainer.childNodes[0]);
    counter--;
  }
  if(start%limit != 0) {
    let newlst = document.createElement("li");
    newlst.setAttribute("class", "link");

    let newlnk='';
    if(getParameterByName('start')==null)
        newlnk = window.location.search+"&start="+0;
    else
        newlnk = window.location.search.replace("start="+start,"start="+0);
    if(i==start)
        newlst.innerHTML = counter;
    else
        newlst.innerHTML = "<a href='"+newlnk+"'>"+counter+"</a>";
    linkcontainer.insertBefore(newlst, linkcontainer.childNodes[0]);
  }
  

  let inc_start= parseInt(start+limit);
  let dec_start= parseInt(start-limit);
  rep = 'start='+start;

  if(dec_start<1) dec_start=0;
  if(inc_start>json.max_results) inc_start=lastHit;

  if(getParameterByName('start')==null) {
      document.getElementById("prev").setAttribute("href", window.location.search + "&start="+dec_start);
    document.getElementById("next").setAttribute("href", window.location.search + "&start="+inc_start);
  }
  else {
      document.getElementById("prev").setAttribute("href", window.location.search.replace(rep,"start="+dec_start));
      document.getElementById("next").setAttribute("href", window.location.search.replace(rep,"start="+inc_start));
  }
  document.getElementById("prev").style.display="inline-block";
  document.getElementById("next").style.display="inline-block";
}

function sendSugg(input, type) {
    console.log(input, type);
    var n = input.indexOf(" ");
    var str;
    if(n!=-1)
      str = input.substr(n+1);
    else
      str=input;
    let requ = "/api/v2/search/suggestions/"+type+"?q="+encodeURIComponent(str);

    var xhttp = new XMLHttpRequest();
    var suggs = document.getElementById("suggs_"+type);
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
           //Action to be performed when the document is ready:
           suggs.innerHTML="";
           suggs.style.display="inline-block";

           console.log(this.responseText);
           var obj = JSON.parse(this.responseText);
           for(let i=0;i<obj.length;i++) {
               var node = document.createElement("a");
               node.onclick=function(){sugg_func(this, type);};
               node.onmousedown=function(){sugg_func(this, type);};
               var textNode = document.createTextNode(obj[i]);
               node.appendChild(textNode);
               suggs.appendChild(node);
           }
        }
    };
    xhttp.open("GET", requ, true);
    xhttp.send();
}

//Searches in the server with the given search options and displays a list of matches
function sendToPage(name) {
	filterOpts["ShowOnlyTextsWithOCR"] = false;
	
	//Get the searched word
	let inputObj = document.getElementById(name);
	let input = inputObj.value;
	//Form the request for the server
	let requ = "search?q="+encodeURIComponent(input).replace(/%20/g, "+");

	//Create the json file with the advanced search options
	if(document.getElementById("fuzzyness").value=="1")
	  requ += "&fuzzyness=1";
	if(document.getElementById("author").value != "")
		requ += "&author="+document.getElementById("author").value;
	if(document.getElementById("publicatedAfter").value != 1700)
	  requ += "&publicatedafter="+document.getElementById("publicatedAfter").value;
	if(document.getElementById("publicatedBefore").value != 2049)
	  requ +=  "&publicatedbefore="+ document.getElementById("publicatedBefore").value;
	if(document.getElementById("limit").value != 10)
	  requ += "&limit="+document.getElementById("limit").value;
	if(document.getElementById("collections").value != "all")
    requ += "&pillars="+document.getElementById("collections").value;
	
  window.location = requ;
}

function sugg_func(x, type) {
  if(type=="corpus") {
    let searchBox = document.getElementById("SpecialSID");
    var r = searchBox.value.indexOf(" ");
    if(r != -1)
      searchBox.value= searchBox.value.substring(0,r+1) + x.textContent;
    else
      searchBox.value=x.textContent;
    searchBox.focus();
  }
  else if(type=="author")
    document.getElementById("author").value=x.textContent;
}

function sugg_disapear(type) {
  document.getElementById("suggs_"+type).style.display="none";
}

function SelectAll(y) {
	let lst = document.getElementsByClassName("booklistinp");
	for(let i = 0; i < lst.length; i++)
		lst[i].checked = y;
}

function CreateBibliography() {
	let lst = document.getElementsByClassName("booklistinp");
	let biblst = "";
	for(let i = 0; i < lst.length; i++) {
		if(lst[i].checked)
		{
      console.log(lst[i].dataset.scanid);
			if(biblst=="")
				biblst=lst[i].dataset.scanid;
			else
				biblst+=","+lst[i].dataset.scanid;
		}
	}
	if(biblst.length==0) {
		alert("No books selected!");
		return;
	}
	window.open("/createbibliography?books="+biblst);
}

function getParameterByName(name, url) {
//COPYRIGHT TO https://stackoverflow.com/questions/901115/how-can-i-get-query-string-values-in-javascript
	if (!url) url = window.location.href;
    name = name.replace(/[\[\]]/g, '\\$&');
    var regex = new RegExp('[?&]' + name + '(=([^&#]*)|&|#|$)'),
      results = regex.exec(url);
    if (!results) 
      return null;
    if (!results[2]) 
      return '';
    return decodeURIComponent(results[2].replace(/\+/g, ' '));
}


window.addEventListener("load", function() {
  GetPillarsFromServer();
  AddListeners();
  ReflectUrlValues();
  GetSearchResultsFromServer();
  ;} ,false);

function AddListeners() {

  // *** ADD LISTENERS *** // 

  //Listeners on author's input field (suggestion for author names)
  let input_authors = document.getElementById("author");
  input_authors.addEventListener("input", function(event){
      sendSugg(document.getElementById("author").value, "author");
  });
  input_authors.addEventListener("keydown", function(event){
      let k = document.getElementById("suggs_author").selec;
      if(k==undefined)
          k=-1;
      if(event.key == "Enter") {
          if(k!=-1) {
              let lst = document.getElementById("suggs_author").children;
              lst[k].click();
          }
      }
      if(event.key=="ArrowUp" || event.key=="ArrowDown") {
          let lst = document.getElementById("suggs_author").children;
          if(k!=-1)
              lst[k].style.background="";
          if(event.key=="ArrowUp") {
              k-=1;
              if(k<0)
                  k=lst.length-1;
          }
          else
              k+=1;
          k=k%lst.length;
          lst[k].style.background = "#ddd";
          event.preventDefault();
          document.getElementById("suggs_author").selec=k;
      }
  });

  //Listeners on main input field
  let input_search= document.getElementById("SpecialSID");
  input_search.focus();

  //Handler to send suggestions after every input.
  input_search.addEventListener("input",function(event){
      sendSugg(document.getElementById("SpecialSID").value, "corpus");
  });

  //Handler to manager 1. Enter to search, 2. select suggestion, 3. next page
  input_search.addEventListener("keydown",function(event){
    
    //1. Press >>Enter<< to search.
    let k = document.getElementById("suggs_corpus").selec;
    if(k==undefined)
      k=-1;
    if(event.key == "Enter") {
      if(k!=-1) {
          let lst = document.getElementById("suggs_corpus").children;
          lst[k].click();
      }
      document.getElementById("srchButton").click();
    }

    //2. Select suggestion.
    else if(event.key=="ArrowUp" || event.key=="ArrowDown") {
      let lst = document.getElementById("suggs_corpus").children;
      if(k!=-1)
        lst[k].style.background="";
      if(event.key=="ArrowUp") {
        k-=1;
        if(k<0)
          k=lst.length-1;
      }
      else
        k+=1;
      k=k%lst.length;
      lst[k].style.background = "#ddd";
      event.preventDefault();
      document.getElementById("suggs_corpus").selec=k;
    }

    //3. Next or previos page with keyboard shortcut.
    let start = 0;
    if(getParameterByName("start") != null)
      start= parseInt(getParameterByName('start'));  
    let limit = 10;
    if(getParameterByName("limit") != null)
      limit = parseInt(getParameterByName('limit'));
    let rep = "start="+start;

    if(event.key == ">") {
      let newStart=start+limit;
      if(getParameterByName('q')!=null && getParameterByName('start')==null)
        window.location = window.location.search+'&start='+limit;
      else
        window.location = window.location.search.replace(rep, "start="+newStart);
    }
    else if(event.key == "<") {
        let newStart = start-limit;
        if(newStart < 1)
          newStart = 0;
        window.location=window.location.search.replace(rep, 'start='+newStart);
    }
  }, true);
}

function ReflectUrlValues() {
  //Modify advanced-search fields with values from url
	if(getParameterByName('fuzzyness')!=null)   
    document.getElementById("fuzzyness").value = 1;
	if(getParameterByName('author')!=null) 
    document.getElementById("author").value=getParameterByName('author');

	if(getParameterByName('publicatedafter')!=null) {
    document.getElementById("publicatedAfter").value = parseInt(getParameterByName
      ('publicatedafter'));
  }
	if(getParameterByName('publicatedbefore')!=null) {
    document.getElementById("publicatedBefore").value = parseInt(getParameterByName
      ('publicatedbefore'));
  }

	if(getParameterByName('pillars')!=null)
    document.getElementById("collections").value=getParameterByName("pillars");
	if(getParameterByName('sorting')!=null) 
    document.getElementById("sorting").value = getParameterByName('sorting');
	if(getParameterByName('limit')!=null) 
    document.getElementById("limit").value = getParameterByName('limit');

  // parse year //
  let after = parseInt(document.getElementById("publicatedAfter").value);
  let before = parseInt(document.getElementById("publicatedBefore").value);
  let year = document.getElementById("year");
  if (after%100 != 0) {
    var opt = document.createElement("option");
    opt.value=2;
    opt.text=after + " till " + before;
    year.add(opt);
    year.value=2;
  }
  else if (before != 2049)
    year.value=after;
  else
    document.getElementById("year").value=0;


  //Add searched words to input field and window title and description 
	let q = getParameterByName('q');
	document.getElementById("SpecialSID").value = q;
	if(q == null)
	  document.title = "CLAS digital book search";
	else
	  document.title = q + " ‒ CLAS digital book search";
	let description = "Search results for \"" + q + "\" from the CLAS digital "
    + "book search collection of over 100,000 pages.";
	document.querySelector('meta[name="description"]').setAttribute("content", 
    description);
}

function GetPillarsFromServer() {
  
  //TODO(fux): Request should NOT go to search server! 
  let requ = "api/v2/search/pillars";

  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    //Add pillars
    if(this.status == 200) {

      //Parse search results from respond
      if (this.responseText == "")
        return;
      var pillars = JSON.parse(this.responseText);

      //Add pillars
      for(let i=0; i<pillars.length; i++) {
        var opt = document.createElement("option");
        opt.value=pillars[i].key;
        opt.text=pillars[i].name;
        document.getElementById("collections").add(opt);
      }
    }
  }
  xhttp.open("GET", requ, true);
  xhttp.send();
}

function GetSearchResultsFromServer() {
 
  console.log("Sending response asking for search results.\n");
  var url = window.location.toString();
  var pos = url.indexOf("?q=");
  if (pos == -1)
    return

  let requ = "api/v2/search" + url.substr(pos, url.length)
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {

    //parse search results from respond
    if (this.responseText == "")
      return;
    var search_results= JSON.parse(this.responseText);

    //Add search results to document
    let obj = document.getElementById("SearchHitList");
    obj.drawablejson = search_results; 
    ShowSelectedValues(obj);
  }
  xhttp.open("GET", requ, true)
  xhttp.send();
}
