#include "style_sheets.h"

namespace styles {
const char slider[] = R"(
QSlider::groove:vertical {
    background: #404244;
    position: absolute;
    left: 4px; right: 4px;
}

QSlider::handle:vertical {
    height: 30px;
    border-radius: 10px;
    background: %1;
    margin: 0 -4px;
}

QSlider::add-page:vertical {
    background: %2;
}
)";

extern const char slider_ir[]=R"(
QSlider::groove:horizontal {
    border: 1px solid #999999;
    height: 18px;
    background: "#404244";
    margin: 2px 0;
}

QSlider::handle:horizontal {
    background: #faa300;
    border: 1px solid #5c5c5c;
    width: 30px;
    margin: -2px 0;
    border-radius: 3px;
}
)";

const char tooltip[]=R"(
<html>
<head/>
​<style>
table, th, td {
  border:1px solid black;
}
</style>
<body style="color:black;">
<table style="width:100%">
  <tr>
    <th>длина волны</th>
    <th>значение</th>
  </tr>
  <tr>
    <td align=center>%1</td>
    <td align=center>%2</td>
  </tr>
</table>
</body>
</html>
)";

}
