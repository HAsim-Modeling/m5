class ChartOptions(object):
    defaults = { 'chart_size' : (8, 4),
                 'figure_size' : [0.1, 0.1, 0.6, 0.85],
                 'title' : None,
                 'fig_legend' : True,
                 'legend' : None,
                 'legend_loc' : 'upper right',
                 'legend_size' : 6,
                 'colormap' : 'jet',
                 'xlabel' : None,
                 'ylabel' : None,
                 'xticks' : None,
                 'xsubticks' : None,
                 'yticks' : None,
                 'ylim' : None,
                 }

    def __init__(self, options=None, **kwargs):
        self.init(options, **kwargs)

    def clear(self):
        self.options = {}

    def init(self, options=None, **kwargs):
        self.clear()
        self.update(options, **kwargs)

    def update(self, options=None, **kwargs):
        if options is not None:
            if not isinstance(options, ChartOptions):
                raise AttributeError, \
                      'attribute options of type %s should be %s' % \
                      (type(options), ChartOptions)
            self.options.update(options.options)

        for key,value in kwargs.iteritems():
            if key not in ChartOptions.defaults:
                raise AttributeError, \
                      "%s instance has no attribute '%s'" % (type(self), key)
            self.options[key] = value

    def __getattr__(self, attr):
        if attr in self.options:
            return self.options[attr]

        if attr in ChartOptions.defaults:
            return ChartOptions.defaults[attr]

        raise AttributeError, \
              "%s instance has no attribute '%s'" % (type(self), attr)

    def __setattr__(self, attr, value):
        if attr in ChartOptions.defaults:
            self.options[attr] = value
        else:
            super(ChartOptions, self).__setattr__(attr, value)

