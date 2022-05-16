# DataExplorer
Specialized data explorer to read csv files (and toustone files like s2p,s3p&s4p), filter and plot data in columns

CSV with comma as separators can be easily read in. The first line with commas is taken as header, skipping all previous lines.
<img width="506" alt="Table" src="https://user-images.githubusercontent.com/14033169/163820263-03d0d7ef-a490-46da-aba1-593e14f83c04.png">

Columns can be selected via context menu for plotting or as sweep variable.
They can also be filtered to the already selected columns or to text in the filter text.

<img width="506" alt="Filter" src="https://user-images.githubusercontent.com/14033169/163820284-611001f9-4793-482a-a6f0-0bde041c27da.png">

The rows can be filtered to selected values in a given row as well via the context menu.
<img width="636" alt="Filter2" src="https://user-images.githubusercontent.com/14033169/163820301-031216a1-008a-4613-a4cb-c48b20404f4b.png">

The selected data can be plotted. The bottom sweep variable is used as x-axis, the rest is split in different plots.
<img width="699" alt="plot" src="https://user-images.githubusercontent.com/14033169/163820319-697a6571-1b87-4db8-bd74-527c252806e2.png">

