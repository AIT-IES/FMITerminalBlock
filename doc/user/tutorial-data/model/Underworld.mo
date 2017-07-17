model Underworld
  Sisyphus sisyphus annotation(
    Placement(visible = true, transformation(origin = {-40, 20}, extent = {{-20, -20}, {20, 20}}, rotation = 0)));
  Hades hades annotation(
    Placement(visible = true, transformation(origin = {40, 20}, extent = {{-20, -20}, {20, 20}}, rotation = 0)));
equation
  connect(hades.verticalSpeed, sisyphus.verticalSpeed) annotation(
    Line(points = {{60, 20}, {80, 20}, {80, -20}, {-80, -20}, {-80, 20}, {-60, 20}}, color = {0, 0, 127}));
  connect(hades.nearlyOnBottom, sisyphus.nearlyOnBottom) annotation(
    Line(points = {{20, 8}, {-20, 8}}, color = {255, 0, 255}));
  connect(hades.nearlyOnTop, sisyphus.nearlyOnTop) annotation(
    Line(points = {{20, 32}, {-20, 32}}, color = {255, 0, 255}));
  annotation(
    uses(Modelica(version = "3.2.2")),
    experiment(StartTime = 0, StopTime = 40, Tolerance = 1e-06, Interval = 0.08),
    __OpenModelica_simulationFlags(jacobian = "coloredNumerical", s = "dassl", lv = "LOG_STATS"),
  Diagram(graphics = {Text(origin = {36, 45}, extent = {{-16, 5}, {24, -5}}, textString = "Hades"), Text(origin = {-41, 45}, extent = {{-19, 5}, {21, -5}}, textString = "Sisyphos"), Text(origin = {-5, -16}, extent = {{-15, 4}, {25, -2}}, textString = "Vertical Speed"), Text(origin = {-6, 29}, extent = {{-14, 3}, {26, -3}}, textString = "nearly on top"), Text(origin = {-2, 2}, extent = {{-18, 4}, {22, -2}}, textString = "nearly on bottom")}));
end Underworld;
