class Sisyphus
  parameter Real maxHeight(start=1000) "The height at which nearlyOnTop is asserted";
  Modelica.Blocks.Interfaces.RealInput verticalSpeed annotation(
    Placement(visible = true, transformation(origin = {-100, 0}, extent = {{-20, -20}, {20, 20}}, rotation = 0), iconTransformation(origin = {-100, 0}, extent = {{-20, -20}, {20, 20}}, rotation = 0)));
  Modelica.Blocks.Interfaces.RealOutput height annotation(
    Placement(visible = true, transformation(origin = {100, 0}, extent = {{-20, -20}, {20, 20}}, rotation = 0), iconTransformation(origin = {100, -3.55271e-15}, extent = {{-20, -20}, {20, 20}}, rotation = 0)));
  Modelica.Blocks.Interfaces.BooleanOutput nearlyOnTop annotation(
    Placement(visible = true, transformation(origin = {100, 60}, extent = {{-20, -20}, {20, 20}}, rotation = 0), iconTransformation(origin = {100, 60}, extent = {{-20, -20}, {20, 20}}, rotation = 0)));
  Modelica.Blocks.Interfaces.BooleanOutput nearlyOnBottom annotation(
    Placement(visible = true, transformation(origin = {100, -60}, extent = {{-20, -20}, {20, 20}}, rotation = 0), iconTransformation(origin = {100, -60}, extent = {{-20, -20}, {20, 20}}, rotation = 0)));
  Modelica.Blocks.Continuous.Integrator ClimbTheHill(y_start = 0)  annotation(
    Placement(visible = true, transformation(origin = {-50, 0}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
  Modelica.Blocks.Logical.GreaterEqualThreshold topGE(threshold = maxHeight) annotation(
    Placement(visible = true, transformation(origin = {50, 60}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
  Modelica.Blocks.Logical.LessEqualThreshold bottomLE(threshold = 0) annotation(
    Placement(visible = true, transformation(origin = {50, -60}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
equation
  connect(bottomLE.y, nearlyOnBottom) annotation(
    Line(points = {{62, -60}, {84, -60}, {84, -60}, {100, -60}}, color = {255, 0, 255}));
  connect(topGE.y, nearlyOnTop) annotation(
    Line(points = {{62, 60}, {84, 60}, {84, 60}, {100, 60}}, color = {255, 0, 255}));
  connect(ClimbTheHill.y, height) annotation(
    Line(points = {{-38, 0}, {88, 0}, {88, 0}, {100, 0}}, color = {0, 0, 127}));
  connect(bottomLE.u, ClimbTheHill.y) annotation(
    Line(points = {{38, -60}, {0, -60}, {0, 0}, {-40, 0}, {-40, 0}, {-38, 0}}, color = {0, 0, 127}));
  connect(topGE.u, ClimbTheHill.y) annotation(
    Line(points = {{38, 60}, {0, 60}, {0, 0}, {-38, 0}, {-38, 0}}, color = {0, 0, 127}));
  connect(verticalSpeed, ClimbTheHill.u) annotation(
    Line(points = {{-100, 0}, {-62, 0}, {-62, 0}, {-62, 0}}, color = {0, 0, 127}));
  annotation(
    uses(Modelica(version = "3.2.2")),
    Icon(graphics = {Line(origin = {-30, -10}, points = {{-50, -50}, {-30, -50}, {70, 70}, {90, 70}}, thickness = 1.5), Line(origin = {-35.8066, -28.2226}, points = {{-2.19338, -5.77735}, {-2.19338, 2.22265}, {-0.193375, 12.2226}, {3.80662, 2.22265}, {3.80662, 2.22265}}), Line(origin = {-34.2071, -7.20711}, points = {{-1.79289, -0.792893}, {2.20711, 3.20711}, {6.20711, 3.20711}, {6.20711, 3.20711}}), Ellipse(origin = {-37, -3}, extent = {{-3, 3}, {3, -3}}, endAngle = 360), Ellipse(origin = {-21, 8}, extent = {{-13, 14}, {13, -14}}, endAngle = 360), Line(origin = {-30.7929, -12.7929}, points = {{-5.20711, -3.20711}, {-5.20711, 4.79289}, {0.792893, 2.79289}, {6.79289, 6.79289}, {6.79289, 6.79289}})}));
end Sisyphus;
