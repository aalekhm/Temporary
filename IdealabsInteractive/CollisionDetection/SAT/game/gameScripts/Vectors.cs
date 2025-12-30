
function MovePoly::drawPolygonEdges(%this) {
      %VERTICES = %this.noOfVertices;
      for(%i = 0; %i < %VERTICES; %i++) {
         %ptA = %this.getName()@"_VERTEX_"@%i;
         %ptB = %this.getName()@"_VERTEX_"@%i+1;
         
         if(%i == %VERTICES - 1) {
            %ptB = %this.getName()@"_VERTEX_0";
         }
      
         %vectorEdge = t2dVectorSub(%ptA.getPosition(), %ptB.getPosition());
         %EDGE_LEN = t2dVectorDistance(%ptA.getPosition(), %ptB.getPosition());
         
         if(isObject(%ptA.edge))
            %ptA.edge.safeDelete();
         %ptA.edge = point.clone(true);//getDot(true);
         
         %ptA.edge.setSize(%EDGE_LEN, 1);
         %ptA.edge.setBlendColor(0, 128, 255, 255);
         
         %edgeNormalized = t2dVectorNormalise(%vectorEdge);
         %centerPos = t2dVectorScale(%edgeNormalized, -%EDGE_LEN/2);      
         %ptA.edge.setPosition(t2dVectorAdd(%ptA.getPosition(), %centerPos));
         
         %angle = calculateAngle(%ptA.getPositionX(), %ptA.getPositionY(), %ptB.getPositionX(), %ptB.getPositionY());
         %ptA.edge.setRotation(%angle);
         
         
         attachToPoly(%this, %ptA);
         attachToPoly(%this, %ptA.edge);
         
         /////////////////////////////////
   }
}

function MovePoly::projectAxis(%this, %CHECK_POLY) {
      %VERTICES = %this.noOfVertices;
      %ALL_AXIS_COLLIDING = true;
      %PENETRATION = 0;
      %PENETRATION_NORMAL = 0 SPC 0;
      
      if($debug) {
            if(!isObject(%this.DEBUG_OBJ)) {
               %this.DEBUG_OBJ = new ScriptObject();
               
               for(%j = 0; %j < %VERTICES; %j++) {
                  if(%j == 0)
                     %this.DEBUG_OBJ.VERTICES_PROJ = getDot(false);
                  else
                     %this.DEBUG_OBJ.VERTICES_PROJ = %this.DEBUG_OBJ.VERTICES_PROJ SPC getDot(false);
               }
            }
      }

      for(%i = 0; %i < %VERTICES; %i++) {
         %ptA = %this@"_VERTEX_"@%i;
         %ptB = %this@"_VERTEX_"@%i+1;
         
         if(%i == %VERTICES - 1) {
            %ptB = %this@"_VERTEX_0";
         }

         %VERTEX_A = %ptA.getPosition();
         %VERTEX_B = %ptB.getPosition();
               
         %VECTOR_EDGE = t2dVectorSub(%VERTEX_A, %VERTEX_B);
         %ANGLE_BETWEEN_VERTICES = calculateAngle(getWord(%VERTEX_A, 0), getWord(%VERTEX_A, 1), getWord(%VERTEX_B, 0), getWord(%VERTEX_B, 1));

         //Axis Drawing...
         //%LHNormal = (-Y, X);//LeftHandNormal
         //%RHNormal = ( Y, X);//RightHandNormal
         
         //Aa, Draws a perpendicular to the edge from %ptA
         %LHNormal = getWord(%VERTEX_A, 0)-getWord(%VECTOR_EDGE, 1) SPC getWord(%VERTEX_A, 1)+getWord(%VECTOR_EDGE, 0);
         %RHNormal = getWord(%VERTEX_A, 0)+getWord(%VECTOR_EDGE, 1) SPC getWord(%VERTEX_A, 1)-getWord(%VECTOR_EDGE, 0);

         %AXIS_PERPENDICULAR_TO_EDGE = t2dVectorSub(%LHNormal, %RHNormal);
         %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE = t2dVectorNormalise(%AXIS_PERPENDICULAR_TO_EDGE);

         //%axis = t2dVectorSub(%LHNormal, %RHNormal);
         //%normalisedAxis = t2dVectorNormalise(%axis);
         
         if($debug) {
            
                  if(!isObject(%ptA.axisPoint))
                     %ptA.axisPoint = point.clone(true);
                  %ptA.axisPoint.setSize(512, 1);
                  %ptA.axisPoint.setBlendColor(128, 0, 128, 255);
                  %ptA.axisPoint.setRotation(90+%ANGLE_BETWEEN_VERTICES);
                  %ptA.axisPoint.setPosition(%VERTEX_A);
                  attachToPoly(%ptA, %ptA.axisPoint);
                  
                  if(!isObject(%ptA.LNormal))
                     %ptA.LNormal = LNormal.clone(true);
                  %ptA.LNormal.setSize(5, 5);
                  %ptA.LNormal.setPosition(%LHNormal);
                  %ptA.LNormal.setBlendColor(255, 255, 0, 255);
                  attachToPoly(%ptA, %ptA.LNormal);
                  
                  if(!isObject(%ptA.RNormal))
                     %ptA.RNormal = RNormal.clone(true);
                  %ptA.RNormal.setSize(5, 5);
                  %ptA.RNormal.setPosition(%RHNormal);
                  %ptA.RNormal.setBlendColor(255, 255, 0, 255);
                  attachToPoly(%ptA, %ptA.RNormal);
         }
         
         %this.MY_LIMITS = %this.projectVertices(%this, %VERTEX_A, %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, 0);
         %this.HIS_LIMITS = %this.projectVertices(%CHECK_POLY, %VERTEX_A, %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, 1);
         
         //echo(%this.MY_LIMITS@" %DOTPROD == "@%this.HIS_LIMITS);
         
         %myMin = getWord(%this.MY_LIMITS, 0);
         %myMax = getWord(%this.MY_LIMITS, 1);
         %hisMin = getWord(%this.HIS_LIMITS, 0);
         %hisMax = getWord(%this.HIS_LIMITS, 1);

         //Overlap can happen either from the LEFT or RIGHT of the POLYGON
         %OVERLAP_LEFT = %hisMax - %myMin;
         %OVERLAP_RIGHT = %hisMin - %myMax;
      
         //echo("%OVERLAP_LEFT == "@%OVERLAP_LEFT@" %OVERLAP_RIGHT == "@%OVERLAP_RIGHT);
         if(%OVERLAP_LEFT < 0 || %OVERLAP_RIGHT > 0) {
            %ALL_AXIS_COLLIDING = false;
            %PENETRATION = 0;
            %PENETRATION_NORMAL = 0 SPC 0;
            break;
         }
         else {
            if(%OVERLAP_LEFT > 0)
               %DEPTH = %OVERLAP_LEFT;
            else
            if(%OVERLAP_RIGHT < 0)
               %DEPTH = %OVERLAP_LEFT;
            
            //%DEPTH = ((%OVERLAP_LEFT < -%OVERLAP_RIGHT)? -%OVERLAP_LEFT : %OVERLAP_RIGHT);
            if(%i == 0) {
               %PENETRATION = %DEPTH;
               %PENETRATION_NORMAL = %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE;
            }
            else {
               if(%DEPTH < %PENETRATION) {
                  %PENETRATION = %DEPTH;
                  %PENETRATION_NORMAL = %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE;
               }
            }

            %ALL_AXIS_COLLIDING = true;
         }
   }
   
   return (%ALL_AXIS_COLLIDING SPC %PENETRATION) NL %PENETRATION_NORMAL;
}

function MovePoly::projectVertices(%this, %POLYGON, %LHNormal, %normalisedAxis, %isMyVerticesProjection) {
      %VERTICES = %POLYGON.noOfVertices;
      %minProd = %maxProd = 0;
      
      for(%i = 0; %i < %VERTICES; %i++) {
         %VERTEX = %POLYGON@"_VERTEX_"@%i;
         
         %vectB = t2dVectorSub(%LHNormal, %VERTEX.getPosition());
         %DOTPROD = t2dVectorDot(%normalisedAxis, %vectB);
         if(%i == 0) {
            %minProd = %maxProd = %DOTPROD;
         }

         %projB = getWord(%normalisedAxis, 0)*%DOTPROD SPC getWord(%normalisedAxis, 1)*%DOTPROD;
         if($debug) {
            %vertexDebugObj = getWord(%this.DEBUG_OBJ.VERTICES_PROJ, %i);
            %vertexDebugObj.setVisible(true);
            %vertexDebugObj.setPosition(t2dVectorSub(%LHNormal, %projB));
            
            if(%isMyVerticesProjection)
               %vertexDebugObj.setBlendColor(0, 255, 0, 255);
            else
               %vertexDebugObj.setBlendColor(255, 0, 0, 255);
               
            //attachToPoly(%POLYGON, %projB_Point);
            //*/
         }

         if(%DOTPROD < %minProd)
            %minProd = %DOTPROD;
         else 
         if(%DOTPROD > %maxProd)
            %maxProd = %DOTPROD;
      }
      
      return (%minProd SPC %maxProd);
      
}

function attachToPoly(%parent, %obj) {
      if(!%obj.getIsMounted()) {
         %localPointVertex = %parent.getLocalPoint(%obj.getPosition());
         %obj.mount(%parent, getWord(%localPointVertex, 0), getWord(%localPointVertex, 1));
      }
}

function MovePoly::projectCircle(%this, %CIRCLE) {
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

function MovePoly::projectCircleEdgeOnVertexAxis(%this, %CIRCLE, %VERTEX_A, %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE) {
      %RADII = %CIRCLE.RADII;
      
      %VERTICES = 2;
      %minProd = %maxProd = 0;
      
      %distance = t2dVectorScale(%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, -%RADII);
      %pointOnCircle = t2dVectorAdd(%distance, %CIRCLE.getPosition());
      %VECTOR_DELTA = t2dVectorSub(%VERTEX_A, %pointOnCircle);
      %DOTPROD_0 = t2dVectorDot(%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, %VECTOR_DELTA);
      %PROJ_POINT = getWord(%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, 0)*%DOTPROD_0 SPC getWord(%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, 1)*%DOTPROD_0;
      
      //echo(%VERTEX_A@" %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE == "@%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE@" "@%PROJ_POINT@" "@%CIRCLE.getPosition()@" %DOTPROD == "@%DOTPROD);
      if($debug) {
            %circleProj0 = getWord(%this.DEBUG_OBJ.CIRCLE_PROJECTIONS, 0);
            %circleProj0.setVisible(true);
            %circleProj0.setBlendColor(0, 255, 0, 255);
            %circleProj0.setPosition(t2dVectorSub(%VERTEX_A, %PROJ_POINT));
      }
      
      %distance = t2dVectorScale(%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, %RADII);
      %pointOnCircle = t2dVectorAdd(%distance, %CIRCLE.getPosition());
      %VECTOR_DELTA = t2dVectorSub(%VERTEX_A, %pointOnCircle);
      %DOTPROD_1 = t2dVectorDot(%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, %VECTOR_DELTA);
      %PROJ_POINT = getWord(%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, 0)*%DOTPROD_1 SPC getWord(%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE, 1)*%DOTPROD_1;
      //echo(%VERTEX_A@" %NORMALISED_AXIS_PERPENDICULAR_TO_EDGE == "@%NORMALISED_AXIS_PERPENDICULAR_TO_EDGE@" "@%PROJ_POINT@" "@%CIRCLE.getPosition()@" %DOTPROD == "@%DOTPROD);

      if($debug) {
            %circleProj1 = getWord(%this.DEBUG_OBJ.CIRCLE_PROJECTIONS, 1);
            %circleProj1.setVisible(true);            
            %circleProj1.setBlendColor(0, 255, 0, 255);
            %circleProj1.setPosition(t2dVectorSub(%VERTEX_A, %PROJ_POINT));
      }

      if(%DOTPROD_0 < %DOTPROD_1)
         return %DOTPROD_0 SPC %DOTPROD_1;
      else 
         return %DOTPROD_1 SPC %DOTPROD_0;
}

function MovePoly::projectVertexOnCircleAxis(%this, %CIRCLE, %VERTEX_ID) {
      %RADII = %CIRCLE.RADII;

      %ptA = %this@"_VERTEX_"@%VERTEX_ID;
      %POLY_VERTEX = %ptA.getPosition();
      
      %DELTA = t2dVectorSub(%POLY_VERTEX, %CIRCLE.getPosition());
      %NORMALISED_DELTA = t2dVectorNormalise(%DELTA);
      
      %DISTANCE = t2dVectorDistance(%POLY_VERTEX, %CIRCLE.getPosition());

      if(%DISTANCE > %RADII)
         return false;
      else {
         %DEPTH = %RADII - %DISTANCE;
         LABEL_vertexPenetrationDepth.setText(%DEPTH);
         
         %finalPos = t2dVectorSub(%CIRCLE.getPosition(), t2dVectorScale(%NORMALISED_DELTA, %DEPTH));
         %CIRCLE.setPosition(%finalPos);
         
         return true;
      }
      
}

function MovePoly::projectVertexOnCircleAxis__1(%this, %CIRCLE, %VERTEX_ID) {
      %RADII = %CIRCLE.RADII;
      %VERTICES = %this.noOfVertices;
      %ALL_AXIS_COLLIDING = false;
////////////////////////////////////////////////////////////
//DRAW 2 circle VERTICES on the axis formed from the center of the circle to the vertex choosen
////////////////////////////////////////////////////////////
/*
      %AXIS_FROM_VERTEX_TO_CIRCLE_CENTER = t2dVectorSub(%VERTEX, %CIRCLE.getPosition());
      %NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER = t2dVectorNormalise(%AXIS_FROM_VERTEX_TO_CIRCLE_CENTER);
      %ANGLE_BETWEEN_VERTEX_AND_CIRCLE_CENTER = calculateAngle(getWord(%VERTEX, 0), getWord(%VERTEX, 1), getWord(%CIRCLE.getPosition(), 0), getWord(%CIRCLE.getPosition(), 1));
      
      %leftPointLength = t2dVectorScale(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, -%RADII);
      %rightPointLength = t2dVectorScale(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, %RADII);
      
      %leftPointOfCircleAlongThisAxis = t2dVectorAdd(%leftPointLength, %CIRCLE.getPosition());
      %rightPointOfCircleAlongThisAxis = t2dVectorAdd(%rightPointLength, %CIRCLE.getPosition());
      %polyMin = %centerMin = t2dVectorDot(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, %leftPointOfCircleAlongThisAxis);
      %polyMax = %centerMax = t2dVectorDot(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, %rightPointOfCircleAlongThisAxis);
      
      %minProd = %maxProd = 0;

      if($debug) {
         %dotRADII = point.clone(true);
         %dotRADII.setSize(5, 5);
         %dotRADII.setBlendColor(0, 0, 255, 255);
         %dotRADII.setPosition(%leftPointOfCircleAlongThisAxis);
         
         %dotRADII = point.clone(true);
         %dotRADII.setSize(5, 5);
         %dotRADII.setBlendColor(0, 0, 255, 255);
         %dotRADII.setPosition(%rightPointOfCircleAlongThisAxis);
         
         %axis = point.clone(true);
         %axis.setSize(512, 1);
         %axis.setBlendColor(0, 128, 0, 255);
         %axis.setRotation(%ANGLE_BETWEEN_VERTEX_AND_CIRCLE_CENTER);
         %axis.setPosition(%VERTEX);
         //attachToPoly(%VERTEX, %VERTEX.axisPointCircle);
      }
*/
//////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

///*
      for(%i = 0; %i < %VERTICES; %i++) {
         %ptA = %this@"_VERTEX_"@%i;
         %POLY_VERTEX = %ptA.getPosition();
         
         %DISTANCE = t2dVectorDistance(%POLY_VERTEX, %CIRCLE.getPosition());

         if(%DISTANCE > %RADII)
            continue;
         
         %DEPTH = %RADII - %DISTANCE;
         %ALL_AXIS_COLLIDING = true;
echo(%i@" %VERTEX == "@%DISTANCE@" "@%RADII);
/*
         %vectB = t2dVectorSub(%POLY_VERTEX, %CIRCLE.getPosition());
         %DOTPROD = t2dVectorDot(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, %vectB);
         
         if($debug) {
            %vertexProj = getWord(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, 0)*%DOTPROD SPC getWord(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, 1)*%DOTPROD;
            %vertexProjDot = point.clone(true);
            %vertexProjDot.setSize(5, 5);
            %vertexProjDot.setBlendColor(255, 128, 0, 255);
            %vertexProjDot.setPosition(t2dVectorAdd(%vertexProj, %CIRCLE.getPosition()));
         }

         if(%DOTPROD < %hisMin)
            %hisMin = %DOTPROD;
         else 
         if(%DOTPROD > %hisMax)
            %hisMax = %DOTPROD;
//*/
      }
/*
echo(%myMin@" == "@%myMax@" *** "@%hisMin@" == "@%hisMax);
      if(   (%hisMin >= %myMin && %hisMin <= %myMax)
            ||
            (%hisMax >= %myMin && %hisMax <= %myMax)
            ||
            (%myMin >= %hisMin && %myMin <= %hisMax)
            ||
            (%myMax >= %hisMin && %myMax <= %hisMax)
      ) {
            %ALL_AXIS_COLLIDING = true;
      }
      else {
            %ALL_AXIS_COLLIDING = false;
      }

      echo("%ALL_AXIS_COLLIDING =================== "@%ALL_AXIS_COLLIDING);
*/
      return (%ALL_AXIS_COLLIDING);
      
}

function MovePoly::projectVertexOnCircleAxis__0(%this, %CIRCLE, %VERTEX) {
      %RADII = %CIRCLE.getWidth()/2;
      
////////////////////////////////////////////////////////////
//DRAW 2 circle edges on the axis formed from the center of the circle to the vertex choosen
////////////////////////////////////////////////////////////
      %AXIS_FROM_VERTEX_TO_CIRCLE_CENTER = t2dVectorSub(%VERTEX, %CIRCLE.getPosition());
      %NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER = t2dVectorNormalise(%AXIS_FROM_VERTEX_TO_CIRCLE_CENTER);
      %ANGLE_BETWEEN_VERTEX_AND_CIRCLE_CENTER = calculateAngle(getWord(%VERTEX, 0), getWord(%VERTEX, 1), getWord(%CIRCLE.getPosition(), 0), getWord(%CIRCLE.getPosition(), 1));
      
      %leftPointLength = t2dVectorScale(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, -%RADII);
      %rightPointLength = t2dVectorScale(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, %RADII);
      
      %leftPointOfCircleAlongThisAxis = t2dVectorAdd(%leftPointLength, %CIRCLE.getPosition());
      %rightPointOfCircleAlongThisAxis = t2dVectorAdd(%rightPointLength, %CIRCLE.getPosition());
      %polyMin = %centerMin = t2dVectorDot(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, %leftPointOfCircleAlongThisAxis);
      %polyMax = %centerMax = t2dVectorDot(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, %rightPointOfCircleAlongThisAxis);
      
      %VERTICES = %this.noOfVertices;
      %minProd = %maxProd = 0;

      if($debug) {
         %dotRADII = point.clone(true);
         %dotRADII.setSize(5, 5);
         %dotRADII.setBlendColor(0, 0, 255, 255);
         %dotRADII.setPosition(%leftPointOfCircleAlongThisAxis);
         
         %dotRADII = point.clone(true);
         %dotRADII.setSize(5, 5);
         %dotRADII.setBlendColor(0, 0, 255, 255);
         %dotRADII.setPosition(%rightPointOfCircleAlongThisAxis);
         
         %axis = point.clone(true);
         %axis.setSize(512, 1);
         %axis.setBlendColor(0, 128, 0, 255);
         %axis.setRotation(%ANGLE_BETWEEN_VERTEX_AND_CIRCLE_CENTER);
         %axis.setPosition(%VERTEX);
         //attachToPoly(%VERTEX, %VERTEX.axisPointCircle);
      }
//////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


///*
      for(%i = 0; %i < %VERTICES; %i++) {
         %ptA = %this@"_VERTEX_"@%i;
         %POLY_VERTEX = %ptA.getPosition();

         //echo("%VERTEX == "@%POLY_VERTEX);
         %vectB = t2dVectorSub(%POLY_VERTEX, %CIRCLE.getPosition());
         %DOTPROD = t2dVectorDot(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, %vectB);
         
         if($debug) {
            %vertexProj = getWord(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, 0)*%DOTPROD SPC getWord(%NORMALISED_AXIS_FROM_VERTEX_TO_CIRCLE_CENTER, 1)*%DOTPROD;
            %vertexProjDot = point.clone(true);
            %vertexProjDot.setSize(5, 5);
            %vertexProjDot.setBlendColor(255, 128, 0, 255);
            %vertexProjDot.setPosition(t2dVectorAdd(%vertexProj, %CIRCLE.getPosition()));
         }

         if(%DOTPROD < %hisMin)
            %hisMin = %DOTPROD;
         else 
         if(%DOTPROD > %hisMax)
            %hisMax = %DOTPROD;
      }

echo(%myMin@" == "@%myMax@" *** "@%hisMin@" == "@%hisMax);
      if(   (%hisMin >= %myMin && %hisMin <= %myMax)
            ||
            (%hisMax >= %myMin && %hisMax <= %myMax)
            ||
            (%myMin >= %hisMin && %myMin <= %hisMax)
            ||
            (%myMax >= %hisMin && %myMax <= %hisMax)
      ) {
            %ALL_AXIS_COLLIDING = true;
      }
      else {
            %ALL_AXIS_COLLIDING = false;
      }
//*/
      echo("%ALL_AXIS_COLLIDING =================== "@%ALL_AXIS_COLLIDING);

      return (%ALL_AXIS_COLLIDING);
      
}

function getDot(%visibility) {
   %pointClone = point.clone(true);
   %pointClone.setSize(5, 5);
   %pointClone.setVisible(%visibility);
   
   return %pointClone;
}
