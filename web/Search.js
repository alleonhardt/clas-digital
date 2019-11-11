let search_parameters = {};
let default_search_parameters = {};


let jsonObj = {}
let lastInput ="";
let filterOpts = {};
let gCurrentSearchId = -1;

function OpenFilter(x)
{
	let retval = x.value;
	let lnk = window.location.search;
	if(getParameterByName('page')!=null)
	{	
	    lnk = window.location.search.replace("&page="+getParameterByName('page'),"");
	}

	let val = getParameterByName(x.id);
	if(val==null)
	    window.location = lnk + "&"+x.id+"="+retval;
	else
	{
	    //10 is standard value of maxresultsperpage and 0 is standard value of the sort algorithm the two filters never collide so we can ignore what filter option it is
	    //and just print the results
	    if(x.value=="10"||x.value=="0")
		    window.location = lnk.replace("&"+x.id+"="+val,"");
	    else
		    window.location = lnk.replace(x.id+"="+val,x.id+"="+retval);
	}
}

function ResetFocus()
{
    document.getElementById("SpecialSID").focus();
}

function SetAdvancedYear(x)
{
    if(x.value==1)
    {
        document.getElementById("CustomDateOpt").style.display = "block";
        document.getElementById("publicatedAfter").focus();
    }
    else if(x.value==0)
    {
        document.getElementById("publicatedAfter").value=1700;
        document.getElementById("publicatedBefore").value=2049;
        document.getElementById("SpecialSID").focus();
    }
    else
    {
        document.getElementById("SpecialSID").focus();
    }
}

function SetCustom()
{
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

function callMetadata(x)
{
    window.location = "/ShowMetadata.html?scanId="+x.dataset.scanid;
}

function ShowSelectedValues(obj)
{
	obj.innerHTML = "";
	let json = obj.drawablejson;


    let after = parseInt(document.getElementById("publicatedAfter").value);
    let before = parseInt(document.getElementById("publicatedBefore").value);
    let year = document.getElementById("year");
    if(after%100 != 0)
    {
        var opt = document.createElement("option");
        opt.value=2;
        opt.text=after + " till " + before;
        year.add(opt);
        year.value=2;
    }
    else if(before != 2049)
        year.value=after;
    
    else
        document.getElementById("year").value=0;


    // *** ADD RESULTS FOOTER *** //
    let maxResultsElement = document.getElementById("maxResults");
    let searchTimeElement = document.getElementById("searchTime");

    maxResultsElement.innerHTML = json.max_results;
    searchTimeElement.innerHTML = json.time;

    let resultBanner = document.getElementById("ResultBanner");
    resultBanner.style.display = "block";

	if(json.max_results==0)
	    return;
	obj.start = json.show_from;
	
	let bookvalues=[];

	for(var i = json.show_from; i < json.show_to; i++)
	{
			bookvalues.push(json.books[i].scanId);
			
			let newList = document.createElement("div");
			newList.id = json.books[i].scanId;
			newList.classList.add("resultbox");
			let inner = "";
			if(!json.books[i].hasocr || (json.books[i].hasocr == undefined))
			{
				inner = "<div class='some_grid_class'><a '";
			}
			else
			{
				inner = "<div class='some_grid_class'><a href='/GetBooks.html";
			}
			inner+= "?query="+document.getElementById("SpecialSID").value;
			inner+="&scanId="+json.books[i].scanId;
			inner+="&fuzzyness="+document.getElementById("fuzzyness").value; 
			inner+="' onclick='return changecolor(this);' style='display: inline-grid;grid-column-start:1;grid-column-end:2;'><h4 style='text-align: left;float:left;'>";
			
			inner+=json.books[i].description+"</h4></a>   "+"<input onclick='this.was_clicked=1;return true;' class='booklistinp' style='width:1rem;height:1rem;display: inline-grid;grid-column-start:2;grid-column-end:3' data-scanid='"+json.books[i].scanId+"' type='checkbox' unchecked></div>";
			inner+="<div class='metadata' onclick='callMetadata(this);' data-scanid='"+json.books[i].scanId+"' style='text-align: left;color: green;font-size: small;margin-top: -0.5rem; cursor: pointer;'>";
			inner+=json.books[i].bibliography;
			inner+="</div><p style='text-align: left;' id='";
			inner+=json.books[i].scanId;
			inner+="-preview'>"+json.books[i].preview+"</p>";
			newList.innerHTML = inner;
			//Show the new link in the search hit list
			obj.appendChild(newList);
	}
	
    let val = "<hr><div class='linkcontainer'>";
    let counter = 1;
    let resultsperpage = 10;
    if(getParameterByName('maxresultsperpage')!=null)
	resultsperpage = parseInt(getParameterByName('maxresultsperpage'));
    for(let i = 0; i < json.max_results; i+=resultsperpage)
    {
	let lnk = window.location.search;
	let rep = getParameterByName('page');
	rep = 'page='+rep;

	let newlnk='';
	if(getParameterByName('page')==null)
	    newlnk = window.location.search+"&page="+counter;
	else
	    newlnk=window.location.search.replace(rep,"page="+counter);
	if(json.page==counter)
	    val+="<a style='margin-right: 1rem;' disabled>"+counter+"</a>";
	  else
	    val+="<a style='margin-right: 1rem;' href='"+newlnk+"'>"+counter+"</a>";
	counter++;
    }
    val+="</div>";
    obj.innerHTML+=val;

    let next_prev = "<div class='prev_next'>";
    let rep = getParameterByName('page');
    rep = 'page='+rep;
    let next_page=json.page+1;
    let prev_page=json.page-1;

    if(prev_page<1) 
       prev_page=1;
    if(next_page>json.max_results/resultsperpage) 
        next_page=json.page;

    let prevlnk = '';
    let nextlnk = '';
    if(getParameterByName('page')==null)
    {
	prevlnk = window.location.search + "&page="+prev_page;
	nextlnk = window.location.search + "&page="+next_page;
    }
    else
    {
	prevlnk=window.location.search.replace(rep,"page="+prev_page);
	nextlnk=window.location.search.replace(rep,"page="+next_page);
    }

    next_prev+="<a style='margin-right: 2rem; padding: 1.5em; margin:0em' href='"+prevlnk+"'>"+"prev"+"</a>";
    next_prev+="<a style='margin-right: 2rem; padding: 1.5em; margin:0em' href='"+nextlnk+"'>"+"next"+"</a>";
    next_prev+="</div>";
    obj.innerHTML+=next_prev;
}

function sendSugg(input)
{
    var n = input.indexOf(" ");
    var str;
    if(n!=-1)
        str = input.substr(n+1);
    else
        str=input;
    let requ = "/api/v1/typeahead/corpus?q="+encodeURIComponent(str);

    var xhttp = new XMLHttpRequest();
    var suggs = document.getElementById("suggs");
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
           // Typical action to be performed when the document is ready:
           suggs.innerHTML="";
           suggs.style.display="inline-block";

           var obj = JSON.parse(this.responseText);
           for(let i=0;i<obj.length;i++)
           {
               var node = document.createElement("a");
               node.onclick=function(){sugg_func(this);};
               node.onmousedown=function(){sugg_func(this);};
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
function sendToPage(name)
{
	filterOpts["ShowOnlyTextsWithOCR"] = false;
	
	//Get the searched word
	let inputObj = document.getElementById(name);
	let input = inputObj.value;
	//Form the request for the server
	let requ = "/search?q="+encodeURIComponent(input).replace(/%20/g, "+");
	console.log(requ);

	//Create the json file with the advanced search options
	if(document.getElementById("fuzzyness").value=="1")
	    requ += "&fuzzyness=1";
	if(document.getElementById("searchIn").value==1)
	    requ += "&title_only=true";
	if(document.getElementById("searchIn").value==2)
	    requ += "&ocr_only=true";
	if(document.getElementById("author").value != "")
		requ += "&author="+document.getElementById("author").value;
	if(document.getElementById("publicatedAfter").value != 1700)
	    requ += "&publicatedafter="+document.getElementById("publicatedAfter").value;
	if(document.getElementById("publicatedBefore").value != 2049)
	    requ +=  "&publicatedbefore="+ document.getElementById("publicatedBefore").value;
	if(document.getElementById("maxresultsperpage").value != 10)
	    requ += "&maxresultsperpage="+document.getElementById("maxresultsperpage").value;
	if(document.getElementById("collections").value != "all")
        requ += "&pillars="+document.getElementById("collections").value;
	
    window.location = requ;
}

function sugg_func(x) 
{
    let searchBox = document.getElementById("SpecialSID");
    var r = searchBox.value.indexOf(" ");
    if(r != -1)
        searchBox.value= searchBox.value.substring(0,r+1) + x.textContent;
    else
        searchBox.value=x.textContent;
    searchBox.focus();
}

function sugg_appear(x) 
{
    console.log(x.textContent);
    let searchBox = document.getElementById("SpecialSID");
    searchBox.value=x.textContent;
    searchBox.focus();
}

function sugg_disapear()
{
    document.getElementById("suggs").style.display="none";
}

function SelectAll(y)
{
	let lst = document.getElementsByClassName("booklistinp");
	for(let i = 0; i < lst.length; i++)
	{
		lst[i].checked = y;
	}
}

function CreateBibliography()
{
	let lst = document.getElementsByClassName("booklistinp");
	let biblst = "";
	for(let i = 0; i < lst.length; i++)
	{
		if(lst[i].checked)
		{
			if(biblst=="")
				biblst=lst[i].dataset.scanid;
			else
				biblst+=","+lst[i].dataset.scanid;
		}
	}
	if(biblst.length==0)
	{
		alert("No books selected!");
		return;
	}
	window.open("/createbibliography?books="+biblst);
}




window.addEventListener("load",function(){initialise('searchlink');ExecuteInitialise();ShowLinks();},false);

function getParameterByName(name, url) {
//COPYRIGHT TO https://stackoverflow.com/questions/901115/how-can-i-get-query-string-values-in-javascript
	if (!url) url = window.location.href;
    name = name.replace(/[\[\]]/g, '\\$&');
    var regex = new RegExp('[?&]' + name + '(=([^&#]*)|&|#|$)'),
        results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, ' '));
}

function ShowLinks()
{
	if(getParameterByName('title_only')!=null)  document.getElementById("searchIn").value = 1; 
	if(getParameterByName('ocr_only')!=null)    document.getElementById("searchIn").value = 2;
	if(getParameterByName('fuzzyness')!=null)   document.getElementById("fuzzyness").value = 1;

	if(getParameterByName('author')!=null) 
        document.getElementById("author").value=getParameterByName('author');

	if(getParameterByName('publicatedafter')!=null) 
        document.getElementById("publicatedAfter").value=parseInt(getParameterByName('publicatedafter'));

	if(getParameterByName('publicatedbefore')!=null) 
        document.getElementById("publicatedBefore").value=parseInt(getParameterByName('publicatedbefore'));
	if(getParameterByName('pillars')!=null)
        document.getElementById("collections").value=getParameterByName("pillars");
	
	
	document.getElementById("SpecialSID").value = getParameterByName('q');
	console.log(ServerDataObj);
	let obj = document.getElementById("SearchHitList");
	obj.drawablejson = ServerDataObj.search;
	let maxResult = 10;
	if(getParameterByName('maxresultsperpage')!=null)
	    maxResult = parseInt(getParameterByName('maxresultsperpage'));
	if(ServerDataObj.search.max_results!=0)
	{
	    obj.drawablejson.show_from = 0;
	    obj.drawablejson.show_to = Math.min(obj.drawablejson.books.length,maxResult);
	}
	ShowSelectedValues(obj);
	if(getParameterByName('f_sort')!=null) document.getElementById("f_sort").value = getParameterByName('f_sort');
	if(getParameterByName('maxresultsperpage')!=null) document.getElementById("maxresultsperpage").value = getParameterByName('maxresultsperpage');
}


function ExecuteInitialise()
    {
    let inp = document.getElementById("SpecialSID");
    inp.focus();
    inp.addEventListener("input",function(event){
            sendSugg(document.getElementById("SpecialSID").value);
    });
    inp.addEventListener("keydown",function(event){

        let k = document.getElementById("suggs").selec;
        if(k==undefined)
            k=-1;
        if(event.key == "Enter") {
            if(k!=-1) {
                let lst = document.getElementById("suggs").children;
                lst[k].click();
            }
            document.getElementById("srchButton").click();
        }

        else if(event.key=="ArrowUp" || event.key=="ArrowDown") {
            let lst = document.getElementById("suggs").children;
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
            document.getElementById("suggs").selec=k;
        }

        let curPage = parseInt(getParameterByName('page'));  
        let nextPage=curPage;

        if(event.key == ">") {
            nextPage = curPage+1;
            if(getParameterByName('q')!=null && getParameterByName('page')==null)
                window.location=window.location.search+'&page=2';
            else
                window.location=window.location.search.replace('page='+curPage, 'page='+nextPage);
        }
        else if(event.key == "<") {
            nextPage = curPage-1;
            if(nextPage < 1)
                nextPage = 1;
            window.location=window.location.search.replace('page='+curPage, 'page='+nextPage);
        }

        
    }, true);

    if(ServerDataObj===undefined)
    {
        console.log("Cant find server data!");
    }
    else
    {
        if(ServerDataObj.pillars != undefined)
        {
            for(let i = 0; i < ServerDataObj.pillars.length; i++)
            {
                var opt = document.createElement("option");
                opt.value=ServerDataObj.pillars[i].key;
                opt.text=ServerDataObj.pillars[i].name;
                document.getElementById("collections").add(opt);

            }
            console.log("Server data found!");
        }
    }
}
