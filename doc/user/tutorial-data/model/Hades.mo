class Hades
  Modelica.Blocks.Logical.RSFlipFlop UpDownStorage(Qini = true) annotation(
    Placement(visible = true, transformation(origin = {0, 0}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
  Modelica.Blocks.Logical.Switch DirectionSelector annotation(
    Placement(visible = true, transformation(origin = {50, 0}, extent = {{-10, 10}, {10, -10}}, rotation = 0)));
  Modelica.Blocks.Sources.Constant SpeedUp(k = 100) annotation(
    Placement(visible = true, transformation(origin = {0, 34}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
  Modelica.Blocks.Sources.Constant SpeedDown(k = -200) annotation(
    Placement(visible = true, transformation(origin = {0, -40}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
  Modelica.Blocks.Interfaces.BooleanInput nearlyOnTop annotation(
    Placement(visible = true, transformation(origin = {-100, 60}, extent = {{-20, -20}, {20, 20}}, rotation = 0), iconTransformation(origin = {-100, 60}, extent = {{-20, -20}, {20, 20}}, rotation = 0)));
  Modelica.Blocks.Interfaces.BooleanInput nearlyOnBottom annotation(
    Placement(visible = true, transformation(origin = {-100, -60}, extent = {{-20, -20}, {20, 20}}, rotation = 0), iconTransformation(origin = {-100, -60}, extent = {{-20, -20}, {20, 20}}, rotation = 0)));
  Modelica.Blocks.Interfaces.RealOutput verticalSpeed annotation(
    Placement(visible = true, transformation(origin = {100, -3.55271e-15}, extent = {{-20, -20}, {20, 20}}, rotation = 0), iconTransformation(origin = {100, -3.55271e-15}, extent = {{-20, -20}, {20, 20}}, rotation = 0)));
equation
  connect(UpDownStorage.R, nearlyOnBottom) annotation(
    Line(points = {{-12, -6}, {-40, -6}, {-40, -60}, {-100, -60}, {-100, -60}}, color = {255, 0, 255}));
  connect(UpDownStorage.S, nearlyOnTop) annotation(
    Line(points = {{-12, 6}, {-40, 6}, {-40, 60}, {-100, 60}, {-100, 60}}, color = {255, 0, 255}));
  connect(DirectionSelector.y, verticalSpeed) annotation(
    Line(points = {{62, 0}, {86, 0}, {86, 0}, {100, 0}}, color = {0, 0, 127}));
  connect(UpDownStorage.Q, DirectionSelector.u2) annotation(
    Line(points = {{12, 6}, {16, 6}, {16, 0}, {38, 0}, {38, 0}}, color = {255, 0, 255}));
  connect(SpeedDown.y, DirectionSelector.u1) annotation(
    Line(points = {{12, -40}, {20, -40}, {20, -8}, {38, -8}, {38, -8}}, color = {0, 0, 127}));
  connect(SpeedUp.y, DirectionSelector.u3) annotation(
    Line(points = {{12, 34}, {20, 34}, {20, 8}, {38, 8}, {38, 8}}, color = {0, 0, 127}));


annotation(
    uses(Modelica(version = "3.2.2")),
    Icon(graphics = {Line(origin = {-9, -30}, points = {{-51, -50}, {-51, -50}, {9, 50}, {69, 50}, {69, 50}}, thickness = 1.5), Line(origin = {19.9428, 30.7661}, points = {{8.05717, -10.7661}, {4.05717, -0.766091}, {6.05717, 7.23391}, {0.0571731, 11.2339}, {-5.94283, 1.23391}, {-7.94283, 3.23391}, {-7.94283, 3.23391}}), Line(origin = {24, 47}, points = {{2, -9}, {-2, 9}, {-2, 9}}), Line(origin = {23, 52.7929}, points = {{-11, -2.79289}, {-7, 1.20711}, {-1, 3.20711}, {7, 3.20711}, {11, -0.792893}, {11, -0.792893}}), Ellipse(origin = {20, 60}, extent = {{-4, 4}, {4, -4}}, endAngle = 360), Ellipse(origin = {-9, 39}, extent = {{-21, 21}, {21, -21}}, endAngle = 360)}));end Hades;
