function initSearch() {

  $('.Input-text').selectize({
    valueField: 'id',
    labelField: 'name',
    searchField: 'name',
    sortField: 'type',
    maxOptions: 10,
    maxItems: 1,
    hideSelected: true,
    create: false,
    options: searchData,
    render: {
      option: function (item, escape) {

        let slug = item.slug || '';
        let by = '';
        if (slug) {
          by = slug.split('/')[0]
        }

        return '<div>' +
          '<span class="title">' +
          '<span>(<small>' + escape(item.type) + '</small>) <strong>' + escape(item.name) + '</strong> <small class="by">' + escape(by) + '</small></span>' +
          '</span>' +
          '</div>';
      }
    },
    onChange: function (id) {
      let nodes = Graph.graphData().nodes;
      var node = nodes.find(function (obj) {
        return obj.id === id;
      });
      if (node) {
        moveCameraToANode(node);
      }
    }
  });

}
