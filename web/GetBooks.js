function ServerGet(filename, okCallback, errorCallback) {
  let sendRequest = new XMLHttpRequest();
  sendRequest.open("GET", filename, true);
  sendRequest.withCredentials = true;
  sendRequest.onreadystatechange = function() {
    if (sendRequest.readyState == 4) {
      if(sendRequest.status == 200)
        okCallback(sendRequest.responseText);
      else
        errorCallback(sendRequest.responseText,sendRequest.status);
    }
  }
  sendRequest.send(null);
}

let gOCRLoaded = false;
let gHitsLoaded = false;
let gOCRError = false;
let gOcrSplittedFile = null;
let gPageLayoutFile = null;
let scanId = '';
let fuzzyness = 0;

function CorrectedScrollIntoView(elem) {
  let rect = document.getElementsByClassName("searchbox")[0].getBoundingClientRect();
  window.scrollTo(0,elem.offsetTop-(rect.bottom+50));
}

function HighlightHitsAndConstructLinkList() {
  let hitlist = document.getElementById("fullsearchhitlist").hitsloaded;

  let ulhitlst = document.getElementById("fullsearchhitlist");
  ulhitlst.innerHTML = "";	//Delete hits if there are any

  if(hitlist) {
	  if(hitlist.books) {
	    //document.getElementsByClassName("topnav")[0].style.display = "block";
	    //document.getElementsByClassName("resizer")[0].style.display = "block";
	    let HighlightList = [];

	    for (let i = 0; i < hitlist.books.length; i++) {
        let link = document.createElement("a");

        let inner = hitlist.books[i].page;
        if(hitlist.is_fuzzy==true)
            inner+="("+hitlist.books[i].words+"); ";
        else
            inner+=" ";
        link.innerHTML = inner;
        link.page = hitlist.books[i].page;
        link.classList.add('hitlinkstyle');
        link.onclick = function() {
          CorrectedScrollIntoView(document.getElementById("uniquepageid"
            + hitlist.books[i].page));
          UpdateViewMode();
          CorrectedScrollIntoView(document.getElementById("uniquepageid"
            + hitlist.books[i].page));
          link.style.color = "purple";
		    }

		    ulhitlst.appendChild(link);

		    HighlightList = hitlist.books[i].words.split(",");

        for(let x = 0; x < HighlightList.length; x++) {
          let thpage = document.getElementById("uniqueocrpage"+hitlist.books[i].page);
          console.log(HighlightList);
          if(thpage!=null) {
            HighlightList[x] = HighlightList[x].replace(new RegExp('o','g'),'[oö]');
            HighlightList[x] = HighlightList[x].replace(new RegExp('u','g'),'[uü]');
            HighlightList[x] = HighlightList[x].replace(new RegExp('a','g'),'[aä]');
            HighlightList[x] = HighlightList[x].replace(new RegExp('s','g'),'[sßſ]');
            thpage.innerHTML=thpage.innerHTML.replace(new RegExp('('+HighlightList[x]+')',
              'gi'),'<mark>$1</mark>');
            }
        }
	    }
	}
	else {
		let link = document.createElement("p");
		link.innerHTML = "No hits found";
		ulhitlst.appendChild(link);
	}
  }
  if(location.hash=="") {
    if(ulhitlst.firstChild!=null)
        ulhitlst.firstChild.click();
    else {
      let elem = document.getElementsByClassName("svgpage");
      if(elem.length>0)
      LoadImageFromSVG(elem[0]);
    }
  }
  else {
    if(location.hash=="#page1") {
      let elem = document.getElementsByClassName("svgpage");
      if(elem.length>0)
      LoadImageFromSVG(elem[0]);
    }
    else {
        CorrectedScrollIntoView(document.getElementById("uniquepageid"
          + location.hash.substr(5)));
    }
  }
}

function UpdateLinkPrev() {
  let val = document.getElementsByClassName("hitlinkstyle");
  let page = window.location.hash.substr(5);
  if(page=="")
    page=0;
  else
    page=parseInt(page);
  for (let i = 0; i < val.length; i++) {
    if(page<=val[i].page) {
      if(!isElementInViewport(val[i]))
        val[i].scrollIntoView();
      return;
    }
  }
}

function UpdateViewMode() {
    let var_arr;
    let var_class = 'pagecontainer';
    let var_class_old = 'pagecontainerFullImg';

    if(document.getElementById("tooglebut").is_full_read)
    {
	var_arr = isPageAlmostVisible('pagecontainer');
	var_class = 'pagecontainerFullImg';
	var_class_old = 'pagecontainer';
    }
    else
	var_arr = isPageAlmostVisible('pagecontainerFullImg');

    for(let i = 0; i < var_arr.length; i++)
	var_arr[i].classList.replace(var_class_old,var_class);
}

function removeElementsByClass(className){
  var elements = document.getElementsByClassName(className);
  while(elements.length > 0){
    elements[0].parentNode.removeChild(elements[0]);
  }
}

function UpdateOneViewMode(elem) {
  let var_class = 'pagecontainer';
  let var_class_old = 'pagecontainerFullImg';

  if(document.getElementById("tooglebut").is_full_read) {
    var_class = 'pagecontainerFullImg';
    var_class_old = 'pagecontainer';
  }

  elem.classList.replace(var_class_old,var_class);
}

function loadOCRFile(ocrtxt) {
  ocrtxt = ocrtxt.replace(new RegExp("<",'g'),"&lt;");
  ocrtxt = ocrtxt.replace(new RegExp(">",'g'),"&gt;");

  let RegSrch = /-----\s(\d+)\s.\s\d+\s-----/g;
  let match;
  gOcrSplittedFile = [];
  gOcrSplittedFile.pageNum = [];
  let maxNum = 0;
  while(match = RegSrch.exec(ocrtxt)) {
    let vl = parseInt(match[1]);
    gOcrSplittedFile.pageNum.push(vl);
    if( vl > maxNum)
      maxNum = vl;
  }
  gOcrSplittedFile.maxPageNum = maxNum;
  let RegSrchSplit = /-----\s\d+\s.\s\d+\s-----/g;
  gOcrSplittedFile.arr = ocrtxt.split(RegSrchSplit);
  gOcrSplittedFile.arr.shift();
  console.log("SPLIT length: "+gOcrSplittedFile.arr.length);
  //Pagelayout file loaded yet? Then construct the pages
  if(gPageLayoutFile!=null)
	  CreatePageLayout();
}

let gTitle = document.title;

function CreatePageLayout() {
  console.log("OCR size: "+gOcrSplittedFile.arr.length);
  console.log("Pages size: "+gPageLayoutFile.pages.length);
  for(let i = 0; i < gOcrSplittedFile.arr.length; i++) {
    let x = document.createElement("p");
    let cont = document.createElement("div");
    let ocrcont = document.createElement("div");
    let anchor = document.createElement("a");
    let pagenumshow = document.createElement("b");
    anchor.id = "page"+gOcrSplittedFile.pageNum[i];

    ocrcont.classList.add("ocrcontainer");

    cont.classList.add('pagecontainer');
    cont.id = "uniquepageid"+gOcrSplittedFile.pageNum[i];
    x.innerHTML = gOcrSplittedFile.arr[i];
    pagenumshow.innerHTML = gOcrSplittedFile.pageNum[i]+"/"+gOcrSplittedFile.maxPageNum;
    pagenumshow.style.position = "absolute";
    pagenumshow.style.bottom = "0.5rem";
    pagenumshow.style.right = "0.5rem";
    pagenumshow.style["font-size"] = 'small';
    x.id = "uniqueocrpage"+gOcrSplittedFile.pageNum[i];
    x.classList.add('ocrpage');
    ocrcont.appendChild(anchor);
    ocrcont.appendChild(x);
    ocrcont.appendChild(pagenumshow);
    cont.appendChild(ocrcont);
    cont.pageNumber = gOcrSplittedFile.pageNum[i];

    document.body.appendChild(cont);
  }

  for(let i = 0; i < gPageLayoutFile.pages.length; i++) {
    let doc = document.getElementById("uniquepageid"+gPageLayoutFile.pages[i].pageNumber);
    if(doc==null) {
	    console.log("Found page without ocr! At : "+gPageLayoutFile.pages[i].pageNumber);
	    let cont = document.createElement("div");
	    let anchor = document.createElement("a");
	    anchor.id = "page"+gPageLayoutFile.pages[i].pageNumber;


	    cont.classList.add('pagecontainerFullImg');
	    cont.id = "uniquepageid"+gPageLayoutFile.pages[i].pageNumber;
	    cont.pageNumber = gPageLayoutFile.pages[i].pageNumber;

	    document.body.appendChild(cont);
	    doc = cont;
	  }

    let lastIndex = gPageLayoutFile.pages[i].file.lastIndexOf("/");
    if(lastIndex==-1)
	    lastIndex=0;
    let svgcode = "<svg class='svgpage' data-width='" 
      + gPageLayoutFile.pages[i].width + "' data-height='"
      + gPageLayoutFile.pages[i].height + "' data-path='"+"/books/" 
      + scanId + "/pages/" + gPageLayoutFile.pages[i].file.substr(lastIndex)
      + "' viewBox='0 0 " + gPageLayoutFile.pages[i].width + " "
      + gPageLayoutFile.pages[i].height + "'></svg>";

    doc.innerHTML +=svgcode;
  }

  let timer = null;
  let currentSize = 0;
  document.body.onscroll = function() {
    let boundrect = document.getElementsByClassName("searchbox")[0]
      .getBoundingClientRect();
    let gNewSize = Math.floor(boundrect.bottom);
    if(currentSize!=gNewSize) {
	    console.log("Resizing ocrs top value!");
	    let lst = document.getElementsByClassName("ocrpage");
	    for(let i = 0; i < lst.length; i++)
		    lst[i].style.top = (gNewSize+17)+"px";
	    lst = document.getElementsByClassName("imgpage");
	    for(let i = 0; i < lst.length; i++)
		    lst[i].style.top = (gNewSize+17)+"px";
	    currentSize = gNewSize;
	  }
	  UpdateViewMode();

    if(timer) 
      window.clearTimeout(timer);

	  timer = window.setTimeout(function(){
	    let arr = isPageVisible('svgpage');

	    if(window.history.replaceState) {
		    let kk = isPageVisible('pagecontainer');
		    if(kk.length==0)
		      kk = isPageVisible('pagecontainerFullImg');
		    if(kk.length>0) {
          let newurl = window.location.search;
          newurl+="#page"+kk[0].pageNumber;
          window.history.replaceState({},null,newurl);
		    }
	    }

	    //UpdateLinkPrev();
	    for(let i = 0; i < arr.length; i++)
		    LoadImageFromSVG(arr[i]);
	  },400);
  }
  document.body.onscroll();

  if(gHitsLoaded)
	  HighlightHitsAndConstructLinkList();
  gOCRLoaded = true;
}

function LoadImageFromSVG(x) {
  let img = document.createElement("img");
  img.style.width = x.dataset.width;
  img.style.height = x.dataset.height;
  img.classList.add('imgpage');
  img.src=x.dataset.path;
  img.onload = function(){ x.parentElement.replaceChild(img,x); }
}

function isPageVisible(whatisvis) {
  let allPages = document.getElementsByClassName(whatisvis);

  let visiblePages = new Array();
  for(let i = 0; i < allPages.length; i++) {
	  if(isElementInViewport(allPages[i]))
	    visiblePages.push(allPages[i]);
  }
  return(visiblePages);
}

function isPageAlmostVisible(whatisvis) {
  let allPages = document.getElementsByClassName(whatisvis);

  let visiblePages = new Array();
  for(let i = 0; i < allPages.length; i++) {
    if(isElementNearViewport(allPages[i]))
      visiblePages.push(allPages[i]);
  }
  return(visiblePages);
}

function isElementNearViewport(el) {
  var rect = el.getBoundingClientRect();

  if(rect.bottom <= -2000)
	  return(false);
  if(rect.top > (window.innerHeight+2000))
	  return(false);
  return(true);
}

function isElementInViewport (el) {
  var rect = el.getBoundingClientRect();
  var optsrect = document.getElementsByClassName("searchbox")[0].getBoundingClientRect();

  // it's definitely outside if bottom < 0
  if(rect.bottom <= 0)
	  return(false);
  if(rect.top > window.innerHeight)
  	return(false);
  if(rect.bottom<(optsrect.bottom+90))
	  return(false);
  return(true);
}


function loadOCRFileError(errortext) {
  gOcrSplittedFile = [];
  
  if(gHitsLoaded)
    HighlightHitsAndConstructLinkList();
  gOCRError = true;

  if(gPageLayoutFile!=null)
	  CreatePageLayout();
}


function loadMetadataFileError(errortxt,state) {
  if(state==403) {
    //document.getElementById("bibliography").innerHTML = 
    //  "Sorry this book is copyright protected you cannot view this...";
    //document.getElementById("bibliography").style.color = "red";
    let elem = document.createElement("div");
    elem.classList.add("copyright");
    elem.innerHTML = "<div><h3>Sorry this book is copyright protected, "
      + "please login to view this book</h3></div>";
    document.body.appendChild(elem);
  }
  else {
	  document.getElementById("bibliography").innerHTML = 
      "Could not load metadata sorry for that :(";
  }
}

function loadPageLayoutFile(layout) {
  gPageLayoutFile = JSON.parse(layout);
  //OCR loaded and splitted yet? Then start building the page!
  if(gOcrSplittedFile!=null)
	  CreatePageLayout();
}

function loadPageLayoutFileError(errortxt) {
  gPageLayoutFile = {pages:[]};
  if(gOcrSplittedFile!=null)
	  CreatePageLayout();
}


function highlightHitsAndLoadHitlistError(text) {
  document.getElementById("fullsearchhitlist").innerHTML = "<li>Could not load hit list sorry for that</li>";
  document.getElementById("fullsearchhitlist").hitsloaded = null;
  if(gOCRLoaded||gOCRError)
	  HighlightHitsAndConstructLinkList();
  gHitsLoaded = true;
}

function highlightHitsAndLoadHitlist(hits) {
  let searchhits = JSON.parse(hits);
  document.getElementById("fullsearchhitlist").hitsloaded = searchhits;
  if(gOCRLoaded||gOCRError)
	  HighlightHitsAndConstructLinkList();
  gHitsLoaded = true;
}

function initialise() {
  if(gGlobalBookId!=undefined)
	  scanId = gGlobalBookId;
  else
    scanId = getParameterByName('scanId');

  let query = decodeURIComponent(getParameterByName('highlight')).replace(' ','+');
  fuzzyness = getParameterByName('fuzzyness');
  if(fuzzyness==null)
	  fuzzyness = 0;

  if(!hasFullscreen(document.documentElement))
	  document.getElementById("fullbut").style.display = 'none';

  ServerGet("/books/"+scanId+"/ocr.txt", loadOCRFile,loadOCRFileError);
  ServerGet("/books/"+scanId+"/readerInfo.json",loadPageLayoutFile,loadMetadataFileError);
  if(query!=null && query!='null') {
    console.log(query);
    ServerGet("/api/v2/search/pages?scanId=" + scanId + '&query=' + query
      +'&fuzzyness='+fuzzyness, 
      highlightHitsAndLoadHitlist, 
      highlightHitsAndLoadHitlistError);
  }
  else
	  document.getElementsByClassName("linknav")[0].style.display = "none";

  ServerGet("/books/"+scanId+"/intern/pages.txt",LoadPreindexingOfPages,
    LoadPreindexingOfPagesError);

  document.getElementById("srchbox").oninput = function() {
	  DoFuzzyMatching(this.value);
  }
  document.getElementById('srchbox').addEventListener("focusout", function(event){
	if(event.relatedTarget)
	  event.relatedTarget.click();
	document.getElementById("fuzzysuggestions").style.visibility="hidden";});
     
  document.getElementById('srchbox').addEventListener("focusin", function(event){
	  if(document.getElementById("fuzzysuggestions").children.length > 0)
	    document.getElementById("fuzzysuggestions").style.visibility="visible";
  });

  document.getElementById("srchbox").addEventListener("keydown",function(event){
    let x = document.getElementById("srchbox").value;
    let k = document.getElementById("fuzzysuggestions").selec;
    if(k==undefined)
	    k = -1;

	  if(event.key == "Enter") {
	    if(k!=-1) {
        let lst = document.getElementsByClassName("fuzzysugslink");
        lst[k].click();
        return;
	    }
	    doCompleteNewSearch();
	  }
	  if(event.key=="ArrowUp" || event.key=="ArrowDown") {
	    let lst = document.getElementsByClassName("fuzzysugslink");
	    if(k!=-1)
		    lst[k].style.background = "";
	    if(event.key=="ArrowUp") {
		    k-=1;
		    if(k < 0)
		      k = lst.length-1;
	    }
	    else
		    k+=1;
	    k = k%lst.length;
	    lst[k].style.background = "#ddd";

	    let docViewTop = document.getElementById("fuzzysuggestions").scrollTop;
	    let docViewBottom = docViewTop + document.getElementById("fuzzysuggestions").
        offsetHeight;

	    let elemTop = lst[k].offsetTop;
	    let elemBottom = elemTop + lst[k].offsetHeight;

	    if(!((elemBottom <= docViewBottom) && (elemTop >= docViewTop))) {
		    if(event.key=="ArrowUp")
		      document.getElementById("fuzzysuggestions").scrollTop = elemTop;
      }
		  else
		    document.getElementById("fuzzysuggestions").scrollTop = elemBottom-document.getElementById("fuzzysuggestions").offsetHeight;
	    event.preventDefault();
	    document.getElementById("fuzzysuggestions").selec = k;
	  }
  });

  document.addEventListener("fullscreenchange", function (event) {
	  if (document.fullscreenElement) {
	    document.getElementById("fullbut").src = 
        "/static/GetBooks/fullscreen_exit-24px.svg";
	  } 
    else 
	    document.getElementById("fullbut").src = "/static/GetBooks/fullscreen-24px.svg";
  });
}



function SelectLastHit() {
    SelectHit(false);
}

function SelectHit(direction) {
    let curr = window.location.hash;
    let integer = parseInt(curr.substr(5));
    let lnks = document.getElementsByClassName("hitlinkstyle");

    if(direction==false)
    {
	for(let i = lnks.length-1; i > -1; i--)
	{
	    if(parseInt(lnks[i].page) < integer)
	    {
		lnks[i].click();
		return;
	    }
	}
	//We are at or before the first hit, so jump to the last hit
	lnks[lnks.length-1].click();
	return;
    }
    else
    {
	for(let i = 0; i < lnks.length; i++)
	{
	    if(parseInt(lnks[i].page) > integer)
	    {
		lnks[i].click();
		return;
	    }
	}
	//We are at the last hit so jump to the first one
	lnks[0].click();
    }
}

function read_mode(x)
{
    if(x.is_full_read)
    {
	x.is_full_read = false;
	x.src = "/static/GetBooks/chrome_reader_mode-24px.svg";
    }
    else
    {
	x.is_full_read = true;
	x.src = "/static/GetBooks/vertical_split-24px.svg";
    }
    UpdateViewMode();
    CorrectedScrollIntoView(document.getElementById("uniquepageid"+location.hash.substr(5)));
}

function tooglefullscreen(x)
{
    if(x.is_full)
    {
	closeFullscreen(document.documentElement);	
	x.is_full = false;
    }
    else
    {
	openFullscreen(document.documentElement);	
	x.is_full = true;
    }
}


function SelectNextHit()
{
    SelectHit(true);
}

let gFuzzyPreindexing = null;
function LoadPreindexingOfPages(txt)
{
    let obj = {};
    let varl = txt.split(/\r?\n/);
    for(let i = 1; i < varl.length; i++)
    {
	let wharl = varl[i].split(";");
	if(wharl.length>1)
	{
	    let kk = wharl[1].split(',');
	    if(obj[wharl[0]] == undefined)
		obj[wharl[0]] = [];
	    for(let i2 = 0; i2 < kk.length; i2++)
		if(kk[i2]!='')
		    obj[wharl[0]].push(parseInt(kk[i2]));
	}
    }
    gFuzzyPreindexing = obj;
}

function LoadPreindexingOfPagesError(err)
{
    console.log("No preindexed speed up of the search");
}

function sort_unique(arr) {
  if (arr.length === 0) return arr;
  arr = arr.sort(function (a, b) { return a*1 - b*1; });
  var ret = [arr[0]];
  for (var i = 1; i < arr.length; i++) { //Start loop at 1: arr[0] can never be a duplicate
    if (arr[i-1] !== arr[i]) {
      ret.push(arr[i]);
    }
  }
  return ret;
}

function perm(xs) {
    //COPYRIGHT TO: https://stackoverflow.com/questions/37579994/generate-permutations-of-javascript-array chacmololvm
  let ret = [];

  for (let i = 0; i < xs.length; i = i + 1) {
    let rest = perm(xs.slice(0, i).concat(xs.slice(i + 1)));

    if(!rest.length) {
      ret.push([xs[i]])
    } else {
      for(let j = 0; j < rest.length; j = j + 1) {
        ret.push([xs[i]].concat(rest[j]))
      }
    }
  }
  return ret;
}

function DoFuzzyMatching(x,iterator,maxHitsPerIteration)
{
    var t0 = performance.now();

    let suggestions = document.getElementById("fuzzysuggestions");
    let results = 0;
    if(iterator==undefined || suggestions.search_done)
    {
	let tmphits = document.getElementsByClassName("tmphit");
	for(let i = 0; i < tmphits.length; i++)
	{
	    tmphits[i].parentNode.insertBefore(tmphits[i].firstChild,tmphits[i]);
	    tmphits[i].parentNode.removeChild(tmphits[i]);
	}
	suggestions.innerHTML = '';
	suggestions.selec = undefined;
	suggestions.scrollTop = 0;
	iterator = 0;
	suggestions.results = 0;
	suggestions.is_fuzzy = true;
	maxHitsPerIteration = 30;
	console.log("New search started");
    }
    else
    {
	results = suggestions.results;
	console.log("Scroll update started");
    }

    if(x=="")
    {
	suggestions.style.visibility = 'hidden';
	return;
    }

    let value = document.getElementsByClassName("ocrpage");

    x = x.toLowerCase();
    x = x.replace(new RegExp(' ','g'),'+');
    x = x.replace(new RegExp('ö','g'),'o');
    x = x.replace(new RegExp('ü','g'),'u');
    x = x.replace(new RegExp('ä','g'),'a');
    x = x.replace(new RegExp('ß','g'),'s');
    x = x.replace(new RegExp('ſ','g'),'s');

    let arr = x.split('+');
    let stringlengthbeforeRegex = arr[0].length;
    let original = arr[0];
    for(let i = 0; i < arr.length; i++)
    {
	arr[i] = arr[i].replace(new RegExp('o','g'),'[oö]');
	arr[i] = arr[i].replace(new RegExp('u','g'),'[uü]');
	arr[i] = arr[i].replace(new RegExp('a','g'),'[aä]');
	arr[i] = arr[i].replace(new RegExp('s','g'),'[sßſ]');
    }
    let new_arr = []
    for(let i = 0; i < arr.length; i++)
	if(arr[i]!='')
	    new_arr.push(arr[i]);
    arr = new_arr;

    let matchstring = new RegExp('((\r\n|\r|\n|.){0,30}('+arr[0]+')(.|\r\n|\r|\n){0,30})','gi');
    if(arr.length > 1&&arr[1].length>0)
    {
	let match = '';
	let index_arr = [];
	for(let i = 0; i < arr.length; i++)
	    index_arr.push(i);
	index_arr = perm(index_arr);

	for(let i = 0; i < index_arr.length; i++)
	{
	    if(match=='')
		match='(';
	    else
		match+='|(';
	    for(let i2 = 0; i2 < index_arr[i].length; i2++)
	    {
		if(i2!=0)
		    match+='(\r\n|\r|\n|.){0,80}';
		match+=arr[index_arr[i][i2]];
	    }
	    match+='.*\\b)';
	}
	matchstring = new RegExp(match,'gi');
    }
    else
    {
	x = arr[0];
	arr = [x];
    }

    if(gFuzzyPreindexing!=null&&suggestions.is_fuzzy&&stringlengthbeforeRegex>2)
    {
	let lkarr = original;
	console.log("SEARCH FOR: "+lkarr);
	let tmpobj = [];
	for(var key in gFuzzyPreindexing)
	{
	    if(key.search(lkarr)!=-1)
	    {
		tmpobj = tmpobj.concat(gFuzzyPreindexing[key]);
	    }
	}
	tmpobj = sort_unique(tmpobj);
	console.log(tmpobj);
	for(let i = 0; i < tmpobj.length; i++)
	{
	    tmpobj[i] = document.getElementById("uniqueocrpage"+tmpobj[i]);
	}
	value = tmpobj;
    }
    else
	suggestions.is_fuzzy = false;

    console.log(value.length);
    console.log(matchstring);
    for(let i = iterator; i < value.length; i++)
    {
	let res = value[i].innerHTML.match(matchstring);
	if(res==null)
	    continue;
	for(let i2 = 0; i2 < res.length; i2++)
	{
	    results+=1;
	    for(let i3 = 0; i3 < arr.length; i3++)
		if(arr[i3]!='')
		    res[i2] = res[i2].replace(new RegExp('('+arr[i3]+')(?![^<>]*>)','gi'),'<mark>$1</mark>');

	    let a = document.createElement("a");
	    a.innerHTML = res[i2]+"<span style='font-weight: bold;float:right;margin-left: 0.8rem;'> S."+value[i].parentNode.parentNode.pageNumber+"</span>";

	    a.page = value[i].parentNode.parentNode.pageNumber;
	    a.word = arr;
	    a.result = results-1;
	    a.tabIndex = 0;
	    a.classList.add("fuzzysugslink");
	    a.onclick = function(){
		let page = document.getElementById("uniqueocrpage"+this.page);
		for(let i4 = 0; i4< this.word.length; i4++)
		    if(this.word[i4]!='')
			page.innerHTML = page.innerHTML.replace(new RegExp('('+this.word[i4]+')(?![^<>]*>)','gi'),'<span class="tmphit">$1</span>');
		CorrectedScrollIntoView(document.getElementById("uniquepageid"+this.page));
		UpdateViewMode();
		CorrectedScrollIntoView(document.getElementById("uniquepageid"+this.page));
		document.getElementById("fuzzysuggestions").style.visibility = 'hidden';
	    }

	    suggestions.appendChild(a);
	}
	if((suggestions.results+maxHitsPerIteration)<results) {
	    suggestions.hits_found = results;
	    suggestions.iterator_i = i;
	    suggestions.search_done = false;
	    suggestions.style.visibility = 'visible';
	    suggestions.onscroll = function(){
		DoFuzzyMatching(document.getElementById("srchbox").value,i,40);
	    }

	    var t1 = performance.now();
	    console.log("Call to doSomething took " + (t1 - t0) + " milliseconds.");
	    return;
	}
    }
    if(results!=0)
	suggestions.style.visibility = 'visible';
    else
	suggestions.style.visibility = 'hidden';

    suggestions.search_done = true;
    suggestions.onscroll = null;
    var t1 = performance.now();
    console.log("Call to doSomething took " + (t1 - t0) + " milliseconds.");
}

function doCompleteNewSearch()
{
	/* ideally use the existing window.location; see:
	   https://github.com/ShadowItaly/clas-digital/issues/172
	*/
	let newurl='';
	if(fuzzyness>0)
    		newurl = "/books/"+scanId+"/pages?highlight="+document.getElementById("srchbox").value+"&fuzzyness="+fuzzyness;
	else
    		newurl = "/books/"+scanId+"/pages?highlight="+document.getElementById("srchbox").value;
/*    if(window.history.pushState)
    {

	window.history.pushState({},document.title,newurl);
	removeElementsByClass("pagecontainer");
	CreatePageLayout();
    }
    else*/
	window.location = newurl;
}

window.addEventListener("load",initialise,false);
document.addEventListener('DOMContentLoaded', function() {
    window.setTimeout(function(){
    let rect = document.getElementById("tpnav").getBoundingClientRect();
    console.log(rect);
    let strval = ""+(Math.floor(rect.bottom)+8)+"px";
    console.log(strval);
    document.getElementsByClassName("searchbox")[0].style.top = strval;
}
    ,400);
}, false);

function unfoldnav(x)
{
    x.classList.remove('unfoldbut');
    x.classList.add('hidebut');
    x.onclick = function() {
	hidenav(x);
    };
    document.getElementsByClassName("searchbox")[0].classList.remove('hidesearchbox');
}

function hidenav(x)
{
    x.classList.remove('hidebut');
    x.classList.add('unfoldbut');
    x.onclick = function() {
	unfoldnav(x);
    };
    document.getElementsByClassName("searchbox")[0].classList.add('hidesearchbox');
}
