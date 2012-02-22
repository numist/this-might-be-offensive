function prep_item(item, neighbor) {
	var color_class = (neighbor.find('.odd_row').length > 0) ? "even_row" : "odd_row";
	$(item).find('div.col').removeClass("odd_row even_row").addClass(color_class);
	return item;
}