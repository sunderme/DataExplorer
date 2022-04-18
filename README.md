# DataExplorer
Specialized data explorer to read csv files, filter and plot data in columns

CSV with comma as separators can be easily read in. The first line with commas is taken as header, skipping all previous lines.
![table][resources/Table.png]

Columns can be selected via context menu for plotting or as sweep variable.
They can also be filtered to the already selected columns or to text in the filter text.
![filter][resources/Filter.png]

The rows can be filtered to selected values in a given row as well via the context menu.
![filter][resources/Filter2.png]

The selected data can be plotted. The bottom sweep variable is used as x-axis, the rest is split in different plots.
![plot][resources/plot.png]
