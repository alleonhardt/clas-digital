let global_timer = null;
let global_start = 0;
let global_count = 0;
let global_factor = 1;

function on_search_async(it)
{
	console.log("Search is going");
	let lst = document.getElementsByClassName("searchable_item");
	let num_items = Math.max(Math.floor((lst.length*global_factor)/50),20);
	global_factor++;

	if(global_start>=lst.length)
	{
		document.getElementById("infoProgress").style.color = "green";
		return;
	}

	lowercase_search = it.value.toLowerCase();
	for(let i = 0; i < num_items; i++)
	{
		if((i+global_start)==lst.length)
		{
			global_start+=i;
			break;
		}
		let children = lst[i+global_start].childNodes;
		let found = false;
		if(it.value=="")
		{
			found=true;
			global_count++;
		}
		else
		{
			for(let inner = 0; inner < children.length; inner++)
			{
				let htm = children[inner].innerHTML;
				if(htm != undefined)
				{
				    if(htm.toLowerCase().search(lowercase_search) != -1)
				    {
					found = true;
					global_count++;
					break;
				    }
				    else if(children[inner].hasAttribute("data-key") && (children[inner].dataset.key == it.value))
				    {
					found = true;
					global_count++;
					break;
				    }
				}
			}
		}
		if(found)
			lst[i+global_start].style.display = "";
		else
			lst[i+global_start].style.display = "none";
	}

	if(it.value!="")
	{
		document.getElementById("num_max_books").innerHTML = lst.length;
		document.getElementById("num_results").innerHTML = global_count;
		document.getElementById("num_filtered").innerHTML = global_start;
	}
	else
	{
		document.getElementById("infoProgress").style.display="none";
	}
	global_start += num_items;
	global_timer = window.setTimeout(function(){on_search_async(it)},150);
}

function on_search(it)
{
	if(global_timer!=null)
	{
		window.clearTimeout(global_timer);
	}
	global_start = 0;
	global_count = 0;
	global_timer = null;
	global_factor = 1;

	document.getElementById("infoProgress").style.display = "block";
	document.getElementById("infoProgress").style.color = "";

	on_search_async(it);
}
