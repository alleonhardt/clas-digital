//Static because it does not matter which user asks to end his session
function DoLogout() {
	document.cookie = "SESSID=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;";
	window.setTimeout(function(){window.location = "/";},300);
}

function initialise(linkforactive) {
	if (typeof(ServerDataObj) == 'undefined') {
		this.ServerDataObj = {};
	}
	else
		this.ServerDataObj = ServerDataObj;

	let vars = document.cookie;
	console.log("Document cookie: "+ document.cookie);
	let pos = vars.indexOf("SESSID=");
	if(pos!=-1)
	{
		let numpos = vars.indexOf(";",pos+7);
		let cookie = "";
		if(numpos==-1)
			cookie = vars.substr(pos+7);
		else
			cookie = vars.substr(pos+7,numpos);

		console.log(cookie);
		let pos2 = cookie.indexOf("::");
		this.ServerDataObj.user = {};
		this.ServerDataObj.user.email = cookie.substr(0,pos2);
		this.ServerDataObj.user.access = parseInt(cookie.substr(pos2+2,cookie.indexOf(":::")));
	}

	if(this.ServerDataObj.user == undefined) {
    console.log("Logged Out");
    document.getElementById("loggedintopnav").style.display = "none";
	    document.getElementById("loggedouttopnav").style.display="block";
	}

	else
	{
    console.log("Logged In");
    document.getElementById("loggedouttopnav").style.display = "none";
    document.getElementById("loggedintopnav").style.display = "flex";
		document.getElementById("LoggedInAs").innerHTML = "Logged in as <strong>"+this.ServerDataObj.user.email+"</strong>";


		//Unlocks all the content that the user can access!
		let list = document.getElementsByClassName("classifiedContent");
		for(var i=0; i < list.length; i++)
		{
			let y = list[i].dataset.acc;
			if((this.ServerDataObj.user.access&y)!=0)
				list[i].style.display="block";
		}
	}
}

function ExecuteLogin()
{
    document.getElementById("LoginForm").onsubmit = function(event){event.preventDefault();}
    document.getElementById("LoginButton").style.display = "none";
    document.getElementById("LoaderButton").style.display = "inherit";
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/login"); 
    xhr.onload = function(event){ 
	if(xhr.status == 200)
	    window.setTimeout(function(){ window.location="/"; }, 300);
	else
	{
	    window.setTimeout(function(){ 
		document.getElementById("LoginButton").style.display = "";
		document.getElementById("LoaderButton").style.display = "";
		let emailtp = document.getElementById("EmailType");
		emailtp.style["background-color"] = "#ffcccb";
		emailtp.value = "";
		emailtp.onclick = function(){emailtp.style.background = "";};
		
		let passwordtp = document.getElementById("PasswordType");
		passwordtp.style["background-color"] = "#ffcccb";
		passwordtp.value = "";
		passwordtp.onclick = function(){passwordtp.style.background = "";};
	    }, 300);
	}
    }; 
    var email = document.getElementById("EmailType").value; 
    var password = document.getElementById("PasswordType").value;
    xhr.send("email="+encodeURIComponent(email)+"&password="+encodeURIComponent(password));
}

window.addEventListener("load",function(){initialise('searchlink')},false);
