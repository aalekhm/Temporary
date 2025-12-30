
   function MovePoly::projectCircle90(%this, %CIRCLE) {
         LABEL_vertexPenetrationDepth.setText("----");
         LABEL_edgePenetrationDepth.setText("----");
         %VERTICES = %this.noOfVertices;
         %IS_COLLIDING = true;

         %CHECK_ID = getVoronoiRegionOrVertex(%this, %CIRCLE.getPosition());
         LABEL_VRID.setText(%CHECK_ID);
         %whatToDo = getWordCount(%CHECK_ID);
         
         if(%whatToDo == 1) {
echo("               CHECKING AXIS "@%CHECK_ID);
            %IS_COLLIDING = %this.projectVertexOnCircleAxis(%CIRCLE, %CHECK_ID);
         }
         if(%whatToDo == 2) {
echo("               CHECKING EDGE "@%CHECK_ID);

               //for(%i = 0; %i < %VERTICES; %i++) {
               
                  %i = getWord(%CHECK_ID, 0);
                  %ptA = %this@"_VERTEX_"@%i;
                  %ptB = %this@"_VERTEX_"@%i+1;
                  
                  if(%i == %VERTICES-1)
                     %ptB = %this@"_VERTEX_"@0;
                           
                  %VERTEX_A = %ptA.getPosition();
                  %VERTEX_B = %ptB.getPosition();
                  
                  %VECTOR_EDGE = t2dVectorSub(%VERTEX_A, %VERTEX_B);
                  %ANGLE_BETWEEN_VERTICES = calculateAngle(getWord(%VERTEX_A, 0), getWord(%VERTEX_A, 1), getWord(%VERTEX_B, 0), getWord(%VERTEX_B, 1));

                  //Axis Drawing...
                  //%LHNormal = (-Y, X);//LeftHandNormal
                  //%RHNormal = ( Y, X);//RightHandNormal
                  
                  //Aa, Draws a perpendicular to the edge from %VERTEX_A
                  %LHNormal = getWord(%VERTEX_A, 0)-getWord(%VECTOR_EDGE, 1) SPC getWord(%VERTEX_A, 1)+getWord(%VECTOR_EDGE, 0);
                  %RHNormal = getWord(%VERTEX_A, 0)+getWord(%VECTOR_EDGE, 1) SPC getWord(%VERTEX_A, 1)-getWord(%VECTOR_EDGE, 0);

                  %AXIS_PERPENDICULAR_TO_EDGE = t2dVectorSub(%LHNormal, %RHNormal);
                  %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE = t2dVectorNormalise(%AXIS_PERPENDICULAR_TO_EDGE);
                  
                  if($debug) {
                           if(!isObject(%this.DEBUG_OBJ)) {
                              %this.DEBUG_OBJ = new ScriptObject();
                              
                              %this.DEBUG_OBJ.axisLine = 0;
                              %this.DEBUG_OBJ.LNormal = 0;
                              %this.DEBUG_OBJ.RNormal = 0;
                              %this.DEBUG_OBJ.CIRCLE_PROJECTIONS = 0 SPC 0;
                              
                              for(%j = 0; %j < %VERTICES; %j++) {
                                 if(%j == 0)
                                    %this.DEBUG_OBJ.VERTICES_PROJ = getDot(false);
                                 else
                                    %this.DEBUG_OBJ.VERTICES_PROJ = %this.DEBUG_OBJ.VERTICES_PROJ SPC getDot(false);
                              }
                              
                              %this.DEBUG_OBJ.CIRCLE_PROJECTIONS = getDot(false) SPC getDot(false);
                           }

                           if(!isObject(%this.DEBUG_OBJ.axisLine))
                              %this.DEBUG_OBJ.axisLine = getDot(true);
                           %this.DEBUG_OBJ.axisLine.setSize(512, 1);
                           %this.DEBUG_OBJ.axisLine.setBlendColor(128, 0, 128, 255);
                           %this.DEBUG_OBJ.axisLine.setRotation(90+%ANGLE_BETWEEN_VERTICES);
                           %this.DEBUG_OBJ.axisLine.setPosition(%VERTEX_A);
                           //attachToPoly(%ptA, %ptA.axisPoint);
                           
                           if(!isObject(%this.DEBUG_OBJ.LNormal))
                              %this.DEBUG_OBJ.LNormal = LNormal.clone(true);
                           %this.DEBUG_OBJ.LNormal.setPosition(%LHNormal);
                           %this.DEBUG_OBJ.LNormal.setBlendColor(255, 255, 0, 255);
                           %this.DEBUG_OBJ.LNormal.setSize(5, 5);
                           //attachToPoly(%ptA, %ptA.LNormal);
                           
                           if(!isObject(%this.DEBUG_OBJ.RNormal))
                              %this.DEBUG_OBJ.RNormal = RNormal.clone(true);
                           %this.DEBUG_OBJ.RNormal.setPosition(%RHNormal);
                           %this.DEBUG_OBJ.RNormal.setBlendColor(255, 255, 0, 255);
                           %this.DEBUG_OBJ.RNormal.setSize(5, 5);
                           //attachToPoly(%ptA, %ptA.RNormal);
                  }
                  
                  ////////////////////////////////////////////////////////////////////
                  /////// VERTEX to CIRCLE center Axis

                  %this.MY_LIMITS = %this.projectVertices(%this, %VERTEX_A, %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, 0);
                  %this.HIS_LIMITS = %this.projectCircleEdgeOnVertexAxis(%CIRCLE, %VERTEX_A, %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE);

                  %myMin = getWord(%this.MY_LIMITS, 0);
                  %myMax = getWord(%this.MY_LIMITS, 1);
                  %hisMin = getWord(%this.HIS_LIMITS, 0);
                  %hisMax = getWord(%this.HIS_LIMITS, 1);

                  //Overlap can happen either from the LEFT or RIGHT of the POLYGON
                  %OVERLAP_LEFT = %hisMax - %myMin;
                  %OVERLAP_RIGHT = %hisMin - %myMax;
                  
                  echo("%OVERLAP_LEFT == "@%OVERLAP_LEFT@" %OVERLAP_RIGHT == "@%OVERLAP_RIGHT);
                  if(%OVERLAP_LEFT < 0 || %OVERLAP_RIGHT > 0)
                     %IS_COLLIDING = false;
                  else {
                     %DEPTH = -((%OVERLAP_LEFT < -%OVERLAP_RIGHT)? %OVERLAP_LEFT : %OVERLAP_RIGHT);
                     LABEL_edgePenetrationDepth.setText(%DEPTH);
                     
                     %circlePos = t2dVectorSub(%CIRCLE.getPosition(), t2dVectorScale(%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, %DEPTH));
                     %CIRCLE.setPosition(%circlePos);

                     //%polyPos = t2dVectorSub(%this.getPosition(), t2dVectorScale(%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, -%DEPTH));
                     //%this.setPosition(%polyPos);

                     %IS_COLLIDING = true;
                  }
         }
         
         return %IS_COLLIDING;
   }