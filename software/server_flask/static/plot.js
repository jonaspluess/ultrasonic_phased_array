/*******************************************************************************
 *
 * Copyright (C) 2016, 2017, Mijnssen Raphael, Pluess Jonas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 ******************************************************************************/

//static device-definitions
var fs = 32000;				//[1/s]
var time_offset = 0.00183;		//[s]
var numb_values_in_buf = 512;

//init global variables
var sonic_speed = 1.0   		//[m/s], updated in index.html
var upsample_factor = 1;
var downsample_factor = 1;


/*******************************************************************************
 * Create a Timeseries-Graph Function
 ******************************************************************************/
function make_timeseries_plot(id_arg){
	var graph = [];		//function return object, has graph.update_plot(data) function
	timeseries = [];	//holds the new sse-data (which is plotted)

	var margin = {top: 40, right: 40, bottom: 60, left: 50};
	//width of html-division element
	var width = document.getElementById(id_arg.substr(1)).offsetWidth;
	var height = 400;

	rec_buf_cnt = 4;	//needs to be initialized here, so x-axis-scale works properly

	//html-division for all the svg-objects
	html_div = d3.select(id_arg).append("div")
				.style("position", "relative")
				.style("height", margin.top + height + margin.bottom + "px");

	html_div_for_tooltip = d3.select(id_arg).append("div")
					.attr("class", "tooltip")
					.style("display", "none");

	var x = d3.scale.linear()
				.range([0, width])
				.domain([time_offset*sonic_speed/2, (time_offset+(rec_buf_cnt*numb_values_in_buf-1)*1.0/fs)*sonic_speed/2]);
	var y = d3.scale.linear()
				.range([height, 0])
				.domain([0, 1]);

	var x_axis = d3.svg.axis()
				.scale(x).orient("bottom")
				.ticks(10)
				.innerTickSize(-height);
	var y_axis = d3.svg.axis()
				.scale(y).orient("left")
				.ticks(5)
				.innerTickSize(-width);

	var zoom = d3.behavior.zoom()
				.x(x)
				.scaleExtent([1, 10])
				.on("zoom", zoomed);

	var svg = html_div.append("svg")
				.attr("width", width + margin.left + margin.right)
				.attr("height", height + margin.top + margin.bottom)
			.append("g")
				.attr("transform", "translate(" + margin.left + ", " + margin.top + ")")
				.call(zoom);

	//invisble svg-rect to capture mouse-events
	var rect = svg.append("rect")
				.attr("width", width)
				.attr("height", height)
				.style("fill", "none")
				.style("pointer-events", "all")
				.on("mouseover", mouseover)
				.on("mousemove", mousemove)
				.on("mouseout", mouseout);

	var container = svg.append("g");

	var line = d3.svg.line()
				.x(function(d,i) { return x((time_offset+i*1.0/fs)*sonic_speed/2) })
				.y(function(d) { return y(d/(8*650)) })
				.interpolate("linear");

	container.append("svg")
				.attr("width",width)
				.attr("height",height)
			.append("path")
				.attr("class", "timeseries_line")
				.attr("d", line(timeseries));
	container.append("g")
				.attr("class", "x axis")
				.attr("transform", "translate(0," + height + ")")
				.call(x_axis);
	container.append("g")
				.attr("class", "y axis")
				.call(y_axis);
	svg.append("text")
					.attr("text-anchor", "middle")
					.attr("transform", "translate("+ (width/2) +","+ (height+(margin.bottom/2))+")")
					.text("Distance [m]");
	svg.append("text")
					.attr("text-anchor", "middle")
					.attr("transform", "translate("+ -(2*margin.left/3) +","+(height/2)+")rotate(-90)")
					.text("Amplitude");
	svg.append("text")
				.attr("class", "label")
				.attr("text-anchor", "middle")
				.attr("transform", "translate("+ (width/2) +","+ -margin.top/2 +")")
				.text("Angle [°]:");


	/***********************************************************************
	 * Update Plot Function
	 **********************************************************************/
	graph.update_plot = function(data){

		rec_buf_cnt_updated = data.rec_buf_cnt;
		if(rec_buf_cnt_updated != rec_buf_cnt){
			rec_buf_cnt = rec_buf_cnt_updated;
			x.domain([time_offset*sonic_speed/2, (time_offset+(rec_buf_cnt*numb_values_in_buf-1)*1.0/fs)*sonic_speed/2]);
		}
		timeseries = data.y_env_downsamp;
		//update scale for y and x values depending on upsampling factor and downsampling factor
		line.y(function(d) { return y(d/(700*32/upsample_factor)) })
		line.x(function(d,i) { return x((time_offset+i*downsample_factor/upsample_factor*1.0/fs)*sonic_speed/2) })

		d3.select(id_arg).select(".x.axis")
					.call(x_axis);
		d3.select(id_arg).select(".y.axis")
					.call(y_axis);
		d3.select(id_arg).select(".timeseries_line")
					.attr("d", line(timeseries));
		d3.select(id_arg).select(".label")
					.text("Angle [°]:" + data.angle);
	}

	function zoomed(){
		svg.select(".x.axis").call(x_axis);
		svg.select(".y.axis").call(y_axis);
		d3.select(id_arg).select(".timeseries_line")
					.attr("d", line(timeseries));
	}

	function mouseover(){
		html_div_for_tooltip.style("display", "inline");
	}

	function mousemove(){
		tooltip_distance = x.invert(d3.mouse(this)[0]).toFixed(2);

		html_div_for_tooltip.text(tooltip_distance + "m")
				.style("left", (d3.event.pageX - 35) + "px")
				.style("top", (d3.event.pageY - 40) + "px");
	}

	function mouseout(){
		html_div_for_tooltip.style("display", "none");
	}

	return graph;
}


/*******************************************************************************
 * Create a Heatmap-Graph Function
 ******************************************************************************/
function make_heatmap_plot_canvas(id_arg, maximum_tracking_obj){
	//function return object
	var graph = [];
	graph.init_plot = null;
	graph.update_plot = null;
	graph.maximum_tracking_obj = maximum_tracking_obj;

	//static plot-definitions
	var margin = {top: 5, right: 40, bottom: 60, left: 50};
	//width of html-division element
	var desired_width = document.getElementById(id_arg.substr(1)).offsetWidth;

	var cell_height = 12;
	var min_numb_rows = 3;

	//updated in graph.update_plot
	var numb_rows = null;
	var numb_cols = null;
	var rec_buf_cnt = null;
	var angle_min = null;
	var angle_max = null;
	var angle = null;
	var cell_width = null;

	//updated in d3-zoom eventlistener
	var transform_state = [];
	transform_state.x = 0;
	transform_state.y = 0;
	transform_state.scale = 1;


	/***********************************************************************
	 * Init Plot Function
	 **********************************************************************/
	graph.init_plot = function(numb_rows, numb_cols){
		graph.x = d3.scale.linear()
					.domain([time_offset*sonic_speed/2, (time_offset+(rec_buf_cnt*numb_values_in_buf-1)*1.0/fs)*sonic_speed/2])
					.range([0, numb_cols*cell_width]);

		graph.y = d3.scale.linear()
					.domain([angle_max+0.5, angle_min-0.5])
					.range([numb_rows*cell_height, 0]);

		graph.x_axis = d3.svg.axis()
					.scale(graph.x)
					.orient("bottom")
					.innerTickSize(-numb_rows*cell_height)
					.ticks(10);

		graph.y_axis = d3.svg.axis()
					.scale(graph.y)
					.orient("left")
					.innerTickSize(-numb_cols*cell_width)
					.ticks((angle_max - angle_min)/2);

		graph.zoom = d3.behavior.zoom()
					.x(graph.x)
					.scaleExtent([1, 10])
					.on("zoom", zoom);

		//html-division for virtual canvas (holding an invisible copy of the image-data)
		html_div_for_virtual_canvas = d3.select(id_arg).append("div");

		//html-division for visible canvas (holding a modified/zoomed version of the image-data, that is shown in browser)
		html_div_for_visible_canvas = d3.select(id_arg).append("div")
					.style("position", "relative")
					.style("height", margin.top + numb_rows*cell_height + margin.bottom + "px");

		html_div_for_tooltip = d3.select(id_arg).append("div")
					.attr("class", "tooltip")
					.style("display", "none");

		graph.virtual_canvas = html_div_for_virtual_canvas.append("canvas")
					.attr("class", "virtual_canvas")
					.attr("width", numb_cols)
					.attr("height", numb_rows)
					.attr("transform", "translate(" + margin.left + "," + margin.top + ")")
					.style("left", margin.left + "px")
					.style("top", margin.top + "px")
					.style("width", numb_cols*cell_width + "px")
					.style("height", numb_rows*cell_height + "px")
					.style("display","none");

		graph.visible_canvas = html_div_for_visible_canvas.append("canvas")
					.attr("class", "visible_canvas")
					.attr("width", numb_cols)
					.attr("height", numb_rows)
					.attr("transform", "translate(" + margin.left + "," + margin.top + ")")
					.style("left", margin.left + "px")
					.style("top", margin.top + "px")
					.style("width", numb_cols*cell_width + "px")
					.style("height", numb_rows*cell_height + "px");

		graph.svg = html_div_for_visible_canvas.append("svg")
					.attr("width", numb_cols*cell_width + margin.left + margin.right)
					.attr("height", numb_rows*cell_height + margin.top + margin.bottom)
				.append("g")
					.attr("transform", "translate(" + margin.left + "," + margin.top + ")");

		//invisble svg-rect to capture mouse-events
		graph.svg.append("rect")
					.attr("width", numb_cols*cell_width)
					.attr("height", numb_rows*cell_height)
					.style("fill", "none")
					.style("pointer-events", "all")
					.on("mouseover", mouseover)
					.on("mousemove", mousemove)
					.on("mouseout", mouseout)
					.call(graph.zoom);

		graph.svg.append("g")
					.attr("class", "x axis")
					.attr("transform", "translate(0," + numb_rows*cell_height + ")")
					.call(graph.x_axis).call(removeZero);

		graph.svg.append("g")
					.attr("class", "y axis")
					.call(graph.y_axis);

		graph.svg.append("text")
					.attr("text-anchor", "middle")
					.attr("transform", "translate("+ -(margin.left/2) +","+(numb_rows*cell_height/2)+")rotate(-90)")
					.text("Angle [°]");

		graph.svg.append("text")
					.attr("text-anchor", "middle")
					.attr("transform", "translate("+ (numb_cols*cell_width/2) +","+ (numb_rows*cell_height+(margin.bottom/2))+")")
					.text("Distance [m]");
	}


	/***********************************************************************
	 * Update Plot Function
	 **********************************************************************/
	graph.update_plot = function(data){
		//update values
		angle = data.angle;
		downsample_factor = data.downsample_factor;
		upsample_factor = data.upsample_factor;
		rec_buf_cnt = data.rec_buf_cnt;

		angle_min = Math.min(data.angle_max,data.angle_min);
		angle_max = Math.max(data.angle_max,data.angle_min);
		numb_rows_updated = Math.abs(angle_max - angle_min) + 1;
		numb_cols_updated = data.y_env_downsamp.length;

		var color = d3.scale.linear()
			.domain([0*32/upsample_factor, 125*32/upsample_factor, 250*32/upsample_factor, 375*32/upsample_factor, 500*32/upsample_factor, 580*32/upsample_factor, 750*32/upsample_factor])
			.range(["#3b8ecc","#4bc3dd","#04e0cf","#87f296","#f7f762","#f90000","#050402"]);

		//init plot, only if conditions have changed
		if((numb_rows != numb_rows_updated)|(numb_cols != numb_cols_updated)){
			//update values
			cell_width = calc_celldim(desired_width,rec_buf_cnt*numb_values_in_buf*upsample_factor/downsample_factor);
			numb_rows = numb_rows_updated;
			numb_cols = numb_cols_updated;
			//remove existing plot
			d3.select(id_arg).selectAll("*").remove();
			//init new plot
			graph.init_plot(numb_rows, numb_cols);
		}

		//update virtual canvas
		graph.virtual_canvas.call(draw_new_line);
		//draw image on visible canvas with modified (zoomed and dragged) context of virtual canvas
		graph.visible_canvas.node().getContext("2d").drawImage(graph.virtual_canvas.node(),-transform_state.x*((rec_buf_cnt*numb_values_in_buf*upsample_factor)/(desired_width*downsample_factor))/transform_state.scale,0,numb_cols/transform_state.scale,numb_rows,0,0,numb_cols, numb_rows);
		//update maxima (will do nothing if no maximum_tracking_obj existent)
		graph.update_maxima(data);

		//update one row on virtual canvas with recieved data
		function draw_new_line(canvas){
			graph.ctx = canvas.node().getContext("2d"),
				image = graph.ctx.createImageData(numb_cols, 1);
			for (var x = 0, p = 0; x < numb_cols; x++) {
				var c = d3.rgb(color(data.y_env_downsamp[x]));
				image.data[p++] = c.r;
				image.data[p++] = c.g;
				image.data[p++] = c.b;
				image.data[p++] = 255;
			}
			graph.ctx.putImageData(image, 0, angle_to_row_nr(angle, angle_min, angle_max));
		}


	}


	/***********************************************************************
	 * Update Visible Maxima on Plot Function
	 **********************************************************************/
	graph.update_maxima = function(data){
		//remove existing plots
		d3.select(id_arg).select(".maxima").remove();

		//send sse-data to maxima-update function and plot them, only if desired number of maximas to track isn't 0
		if(graph.maximum_tracking_obj.n != 0){
			if(data){
				graph.maximum_tracking_obj.update_maxima(data, numb_rows);
			}

			//create svg group for maxima vector elements
			graph.svg_maximas = graph.svg.append("g").attr("class", "maxima").append("svg")
							.attr("width",numb_cols*cell_width)
							.attr("height",numb_rows*cell_height);

			//draw maxima
			for(var i = 0; i < graph.maximum_tracking_obj.n; i++){
				if(graph.maximum_tracking_obj.data[i].maximum != null){
					graph.svg_maximas.append("circle")
								.attr("cx", graph.x(graph.maximum_tracking_obj.data[i].distance))
								.attr("cy", graph.y(graph.maximum_tracking_obj.data[i].angle))
								.attr("r", 8)
								.style("fill","none")
								.style("stroke","black")
								.style("storke-width","5px");
					graph.svg_maximas.append("text")
								.attr("text-anchor", "middle")
								.attr("transform", "translate("+ (graph.x(graph.maximum_tracking_obj.data[i].distance)) + ","+ (graph.y(graph.maximum_tracking_obj.data[i].angle) + 4) +")")
								.text((i + 1));
					graph.svg_maximas.append("text")
								.attr("text-anchor", "middle")
								.attr("transform", "translate("+ (graph.x(graph.maximum_tracking_obj.data[i].distance) + 10) + ","+ (graph.y(graph.maximum_tracking_obj.data[i].angle) - 14) +")")
								.text(graph.maximum_tracking_obj.data[i].distance.toFixed(2) + "m|" + graph.maximum_tracking_obj.data[i].angle + "°");
				}
			}
		}
	}

	function zoom(){
		//update transform states
		transform_state.scale = d3.event.scale;
		transform_state.x = d3.event.translate[0];
		transform_state.y = d3.event.translate[1];

		graph.svg.select(".x.axis").call(graph.x_axis);

		//draw image on visible canvas with modified (zoomed and dragged) context of virtual canvas
		graph.visible_canvas.node().getContext("2d").clearRect(0, 0, numb_cols, numb_rows);
		graph.visible_canvas.node().getContext("2d").drawImage(graph.virtual_canvas.node(),-transform_state.x*((rec_buf_cnt*numb_values_in_buf*upsample_factor)/(desired_width*downsample_factor))/transform_state.scale,0,numb_cols/transform_state.scale,numb_rows,0,0,numb_cols, numb_rows);

		//draw maxima, only if maximum_tracking-object exists
		if(graph.maximum_tracking_obj){
			graph.update_maxima();
		}
	}

	function mouseover(){
		html_div_for_tooltip.style("display", "inline");
	}

	function mousemove(){
		tooltip_distance = graph.x.invert(d3.mouse(this)[0]).toFixed(2);
		tooltip_angle = Math.round(graph.y.invert(d3.mouse(this)[1]));

		if(tooltip_angle == -0){
			tooltip_angle = 0;
		}

		html_div_for_tooltip.text(tooltip_distance + "m" + ", " + tooltip_angle + "°")
				.style("left", (d3.event.pageX - 35) + "px")
				.style("top", (d3.event.pageY - 40) + "px");
	}

	function mouseout(){
		html_div_for_tooltip.style("display", "none");
	}

	//helper functions
	function angle_to_row_nr (angle, angle_limit_1, angle_limit_2){
		angle_min = Math.min(angle_limit_1, angle_limit_2);
		row_nr = Math.abs(angle_min - angle);
		return parseInt(row_nr);
	}

	function calc_celldim(length_in_pxl, num_cells){
		return (length_in_pxl/num_cells);
	}

	function removeZero(axis){
		axis.selectAll("g").filter(function(d) { return !d; }).remove();
	}
	return graph;
}


/*******************************************************************************
 * Maximum Tracking Function
 ******************************************************************************/
function start_maximum_tracking(numb_of_maxima){
	//settings
	var numb_peaks_per_array = 6;				//how many peaks are saved per new data
	var min_euclidean_metric_allowed = 3;		//minimal euclidean distance allowed
	var forget_factor = 2;						//how long a stored maximum is kept: (forget_factor * number_of_rows) data-updates

	//return object: contains all information about the recent maxima and has the function "maximum_tracking.update_maxima(data)"
	var maximum_tracking = [];
	maximum_tracking.now = [];				//contains objects for each new maximum (from new sse-data)
	maximum_tracking.data = [];				//conatains objects for each stored maximum
	maximum_tracking.n = numb_of_maxima;	//limit for number of stored maximums

	//init data structure: "maximum_tracking.data"
	for(var i = 0; i < maximum_tracking.n; i++){
		maximum_tracking.data.push({maximum: null, distance: null, angle: null, euclidean_metric: null, minimal_euclidean_metric: null, counter: 0, dispose: null});
	}
	//init data structure: "euclidean_metric" and attach it to each maximum, it stores the distances to all other saved maxima
	for(var i = 0; i < maximum_tracking.n; i++){
			var euclidean_metric = []
			for(var j = 0; j < maximum_tracking.n; j++){
				euclidean_metric.push({value: null})
			}
			maximum_tracking.data[i]["euclidean_metric"] = euclidean_metric;
	}


	/***********************************************************************
	 * Recalculate all Maxima (callback function)
	 **********************************************************************/
	maximum_tracking.update_maxima = function(data, numb_rows){
		//update after how many updates a maximums gets deleted
		forget_after_n_iter = numb_rows * forget_factor

		//find n peaks in new data and save their information (distance, angle and maximum-value) in maximum_tracking.now
		maximum_tracking.now = find_n_peaks_in_data(data,numb_peaks_per_array);

		//update the maximum_tracking.data datastructure if the new values are big enough
		for(var j = 0; j < numb_peaks_per_array; j++){
			for(var i = 0; i < maximum_tracking.n; i++){
				if((maximum_tracking.data[i].maximum < maximum_tracking.now[j].maximum)){
					maximum_tracking.data[maximum_tracking.n - (j + 1)]["maximum"] = maximum_tracking.now[j].maximum;
					maximum_tracking.data[maximum_tracking.n - (j + 1)]["distance"] = maximum_tracking.now[j].distance;
					maximum_tracking.data[maximum_tracking.n - (j + 1)]["angle"] = maximum_tracking.now[j].angle;
					maximum_tracking.data[maximum_tracking.n - (j + 1)]["counter"] = 0;
					break;
				}
			}
		}

		//calculate the minimal euclidean-metric for all maxima compared to all other saved maxima (saved in maximum_tracking.data[i].euclidean_metric)
		for(var i = 0; i < maximum_tracking.n; i++){
			for(var j = 0; j < maximum_tracking.n; j++){
				if(i != j){
					var x = 50 * Math.abs(maximum_tracking.data[i].distance - maximum_tracking.data[j].distance);
					var y = 0.5 * Math.abs(maximum_tracking.data[i].angle - maximum_tracking.data[j].angle);
					maximum_tracking.data[i]["euclidean_metric"][j]["value"] = Math.sqrt(Math.pow(x,2)+Math.pow(y,2));
				}else{
					maximum_tracking.data[i]["euclidean_metric"][j]["value"] = null;
				}
			}
			//find the minimal euclidean_metric of current (i-th) maximum, while not considering null-values in the array
			maximum_tracking.data[i].minimal_euclidean_metric = Math.min.apply(Math,maximum_tracking.data[i]["euclidean_metric"].map(function(object){
				if(object.value != null){
					return object.value;
				}else{
					return Infinity
				};}))
		}
		//set the disposal-flags for i-th maximum (marking a maximum to be deleted)
		for(var i = 0; i < maximum_tracking.n; i++){
			//increment counter of saved maxima, so old values can be kicked out
			maximum_tracking.data[i]["counter"] = maximum_tracking.data[i]["counter"] + 1;

			//kick out if counter is too high
			if((maximum_tracking.data[i].counter > forget_after_n_iter)){
				maximum_tracking.data[i].dispose = true;
			//keep values if there arent any close neighbours
			}else if((maximum_tracking.data[i].minimal_euclidean_metric > min_euclidean_metric_allowed)){
				maximum_tracking.data[i].dispose = false;
			//we have close neigbours! kick the value out if it is an old value
			}else if(maximum_tracking.data[i].counter > (numb_rows*6/5)){
				maximum_tracking.data[i].dispose = true;
			//we have close neigbours and the value is fresh! now we have to examine the local area around the maximum
			}else if(maximum_tracking.data[i].dispose == null){
				//search the local area around i-th maximum and set disposal flags for the local maximums
				examine_close_neighbours(min_euclidean_metric_allowed, i);
			}
		}

		//delete values marked with the dispose flag
		for(var i = 0; i < maximum_tracking.n; i++){
			if(maximum_tracking.data[i].dispose == true){
				maximum_tracking.data[i]["counter"] = 0;
				maximum_tracking.data[i]["maximum"] = null;
				maximum_tracking.data[i]["distance"] = null;
				maximum_tracking.data[i]["angle"] = null;
			}
			maximum_tracking.data[i]["dispose"] = null;
		}

		//sort all stored maximums (they are going to be plottet in this order)
		maximum_tracking.data.sort(function(a, b) { return b.maximum - a.maximum; });
	}

	//helper function: returns n objects for the n largest peaks in the new data-array (sent per SSE), with corresponding distances and angles
	function find_n_peaks_in_data(data,n){
		var all_peaks_in_arr = [];
		data.y_env_downsamp.forEach(function(val,index,arr){
			if((arr[index-1] < val) && (arr[index+1] < val)){
				all_peaks_in_arr.push({distance : ((time_offset+index*1.0/fs)*sonic_speed/2), maximum: val, angle: data.angle});
			}
		});
		all_peaks_in_arr.sort(function(a, b) { return b.maximum - a.maximum; });
		return all_peaks_in_arr.slice(0,n)
	}

	//helper function: examine surrounding close maximums within the chosen radius around the current maximum i and set disposal flags
	function examine_close_neighbours(radius,index){
		for(var j = 0; j < maximum_tracking.n; j++){
			if((maximum_tracking.data[index]["euclidean_metric"][j]["value"] < radius) & (index != j)){
				if(maximum_tracking.data[index].maximum > maximum_tracking.data[j].maximum){
					maximum_tracking.data[j].dispose = true;
				}else{
					maximum_tracking.data[index].dispose = true;
					break;
				}
			}
		}
	}

	return maximum_tracking
}


/*******************************************************************************
 * SSE Eventlistener
 ******************************************************************************/
function sse_listener(){
	// parameters: variable amount of callback functions
	callback_functions = arguments;

	if(typeof(EventSource) != "undefined"){
		var source = new EventSource("/subscribe");
		//eventlistener for new sse-message
		source.onmessage = function(event){
			var data = JSON.parse(event.data);
			for (i = 0; i < callback_functions.length; i++){
				//call all callback functions with recieved new data
				callback_functions[i](data);
			}
		}
	}else{
		document.getElementById("result").innerHTML = "Sorry, your browser doesn't support server-sent events.";
	}
}
