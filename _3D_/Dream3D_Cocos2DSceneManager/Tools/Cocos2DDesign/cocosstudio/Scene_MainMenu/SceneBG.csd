<GameFile>
  <PropertyGroup Name="SceneBG" Type="Scene" ID="20e775ef-3d30-4a20-9cc8-51de9b2d74ec" Version="3.10.0.0" />
  <Content ctype="GameProjectContent">
    <Content>
      <Animation Duration="362" Speed="1.0000" ActivedAnimationName="animation_SinglePlayer">
        <Timeline ActionTag="-540224482" Property="VisibleForFrame">
          <BoolFrame FrameIndex="0" Tween="False" Value="True" />
        </Timeline>
        <Timeline ActionTag="-1344135665" Property="VisibleForFrame">
          <BoolFrame FrameIndex="0" Tween="False" Value="True" />
          <BoolFrame FrameIndex="60" Tween="False" Value="True" />
          <BoolFrame FrameIndex="120" Tween="False" Value="False" />
          <BoolFrame FrameIndex="181" Tween="False" Value="False" />
          <BoolFrame FrameIndex="302" Tween="False" Value="False" />
        </Timeline>
        <Timeline ActionTag="-1344135665" Property="Scale">
          <ScaleFrame FrameIndex="302" X="1.0000" Y="1.0000">
            <EasingData Type="0" />
          </ScaleFrame>
        </Timeline>
        <Timeline ActionTag="-1344135665" Property="Position">
          <PointFrame FrameIndex="302" X="320.0000" Y="480.0000">
            <EasingData Type="0" />
          </PointFrame>
        </Timeline>
        <Timeline ActionTag="-1344135665" Property="RotationSkew">
          <ScaleFrame FrameIndex="302" X="0.0000" Y="0.0000">
            <EasingData Type="0" />
          </ScaleFrame>
        </Timeline>
        <Timeline ActionTag="-1344135665" Property="Alpha">
          <IntFrame FrameIndex="0" Value="0">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="60" Value="255">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="120" Value="0">
            <EasingData Type="0" />
          </IntFrame>
        </Timeline>
        <Timeline ActionTag="-494358391" Property="VisibleForFrame">
          <BoolFrame FrameIndex="0" Tween="False" Value="False" />
          <BoolFrame FrameIndex="60" Tween="False" Value="False" />
          <BoolFrame FrameIndex="120" Tween="False" Value="True" />
          <BoolFrame FrameIndex="181" Tween="False" Value="True" />
          <BoolFrame FrameIndex="241" Tween="False" Value="False" />
          <BoolFrame FrameIndex="242" Tween="False" Value="True" />
          <BoolFrame FrameIndex="302" Tween="False" Value="True" />
          <BoolFrame FrameIndex="362" Tween="False" Value="True" />
        </Timeline>
        <Timeline ActionTag="-494358391" Property="Scale">
          <ScaleFrame FrameIndex="302" X="1.0000" Y="1.0000">
            <EasingData Type="0" />
          </ScaleFrame>
        </Timeline>
        <Timeline ActionTag="-494358391" Property="Position">
          <PointFrame FrameIndex="302" X="320.0000" Y="480.0000">
            <EasingData Type="0" />
          </PointFrame>
        </Timeline>
        <Timeline ActionTag="-494358391" Property="RotationSkew">
          <ScaleFrame FrameIndex="302" X="0.0000" Y="0.0000">
            <EasingData Type="0" />
          </ScaleFrame>
        </Timeline>
        <Timeline ActionTag="-494358391" Property="Alpha">
          <IntFrame FrameIndex="120" Value="0">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="181" Value="255">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="241" Value="0">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="242" Value="255">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="362" Value="255">
            <EasingData Type="0" />
          </IntFrame>
        </Timeline>
        <Timeline ActionTag="312679254" Property="VisibleForFrame">
          <BoolFrame FrameIndex="0" Tween="False" Value="False" />
          <BoolFrame FrameIndex="60" Tween="False" Value="False" />
          <BoolFrame FrameIndex="181" Tween="False" Value="False" />
          <BoolFrame FrameIndex="241" Tween="False" Value="True" />
          <BoolFrame FrameIndex="302" Tween="False" Value="True" />
        </Timeline>
        <Timeline ActionTag="312679254" Property="Alpha">
          <IntFrame FrameIndex="241" Value="0">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="302" Value="255">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="362" Value="0">
            <EasingData Type="0" />
          </IntFrame>
        </Timeline>
      </Animation>
      <AnimationList>
        <AnimationInfo Name="animation_SinglePlayer" StartIndex="0" EndIndex="0">
          <RenderColor A="255" R="250" G="240" B="230" />
        </AnimationInfo>
        <AnimationInfo Name="animation_Battles_In" StartIndex="0" EndIndex="60">
          <RenderColor A="255" R="245" G="255" B="250" />
        </AnimationInfo>
        <AnimationInfo Name="animation_Battles_Out" StartIndex="60" EndIndex="120">
          <RenderColor A="255" R="175" G="238" B="238" />
        </AnimationInfo>
        <AnimationInfo Name="animation_Tournaments_In" StartIndex="121" EndIndex="181">
          <RenderColor A="255" R="255" G="245" B="238" />
        </AnimationInfo>
        <AnimationInfo Name="animation_Tournaments_Out" StartIndex="181" EndIndex="241">
          <RenderColor A="255" R="218" G="165" B="32" />
        </AnimationInfo>
        <AnimationInfo Name="animation_Retro_In" StartIndex="242" EndIndex="302">
          <RenderColor A="255" R="240" G="255" B="255" />
        </AnimationInfo>
        <AnimationInfo Name="animation_Retro_Out" StartIndex="302" EndIndex="362">
          <RenderColor A="255" R="255" G="20" B="147" />
        </AnimationInfo>
      </AnimationList>
      <ObjectData Name="SceneBG" CustomClassName="SceneBG" Tag="5269" ctype="GameNodeObjectData">
        <Size X="640.0000" Y="960.0000" />
        <Children>
          <AbstractNodeData Name="Image_BG" ActionTag="-540224482" Tag="385" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" TopEage="187" BottomEage="187" Scale9OriginY="187" Scale9Width="1" Scale9Height="195" ctype="ImageViewObjectData">
            <Size X="640.0000" Y="960.0000" />
            <Children>
              <AbstractNodeData Name="Image_Left" ActionTag="-1260359833" Tag="185" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" RightMargin="320.0000" LeftEage="52" RightEage="52" TopEage="187" BottomEage="187" Scale9OriginX="52" Scale9OriginY="187" Scale9Width="56" Scale9Height="195" ctype="ImageViewObjectData">
                <Size X="320.0000" Y="960.0000" />
                <AnchorPoint ScaleX="1.0000" />
                <Position X="320.0000" />
                <Scale ScaleX="1.0000" ScaleY="1.0000" />
                <CColor A="255" R="255" G="255" B="255" />
                <PrePosition X="0.5000" />
                <PreSize X="0.5000" Y="1.0000" />
                <FileData Type="Normal" Path="Common/BG_SinglePlayer_2.png" Plist="" />
              </AbstractNodeData>
              <AbstractNodeData Name="Image_Right" ActionTag="-1918898351" Tag="186" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" LeftMargin="320.0000" LeftEage="52" RightEage="52" TopEage="187" BottomEage="187" Scale9OriginX="52" Scale9OriginY="187" Scale9Width="56" Scale9Height="195" ctype="ImageViewObjectData">
                <Size X="320.0000" Y="960.0000" />
                <AnchorPoint />
                <Position X="320.0000" />
                <Scale ScaleX="1.0000" ScaleY="1.0000" />
                <CColor A="255" R="255" G="255" B="255" />
                <PrePosition X="0.5000" />
                <PreSize X="0.5000" Y="1.0000" />
                <FileData Type="Normal" Path="Common/BG_SinglePlayer_2.png" Plist="" />
              </AbstractNodeData>
            </Children>
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="320.0000" Y="480.0000" />
            <Scale ScaleX="1.0000" ScaleY="1.0000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.5000" />
            <PreSize X="1.0000" Y="1.0000" />
            <FileData Type="Normal" Path="Common/BG_SinglePlayer.png" Plist="" />
          </AbstractNodeData>
          <AbstractNodeData Name="Image_Battles_BG" ActionTag="-1344135665" Alpha="0" Tag="1521" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" TopEage="187" BottomEage="187" Scale9OriginY="187" Scale9Width="1" Scale9Height="195" ctype="ImageViewObjectData">
            <Size X="640.0000" Y="960.0000" />
            <Children>
              <AbstractNodeData Name="Image_Left" ActionTag="-453946653" Tag="187" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" RightMargin="320.0000" LeftEage="52" RightEage="52" TopEage="187" BottomEage="187" Scale9OriginX="52" Scale9OriginY="187" Scale9Width="56" Scale9Height="195" ctype="ImageViewObjectData">
                <Size X="320.0000" Y="960.0000" />
                <AnchorPoint ScaleX="1.0000" />
                <Position X="320.0000" />
                <Scale ScaleX="1.0000" ScaleY="1.0000" />
                <CColor A="255" R="255" G="255" B="255" />
                <PrePosition X="0.5000" />
                <PreSize X="0.5000" Y="1.0000" />
                <FileData Type="Normal" Path="Common/BG_Battle_2.png" Plist="" />
              </AbstractNodeData>
              <AbstractNodeData Name="Image_Right" ActionTag="-927747300" Tag="188" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" LeftMargin="320.0000" LeftEage="52" RightEage="52" TopEage="187" BottomEage="187" Scale9OriginX="52" Scale9OriginY="187" Scale9Width="56" Scale9Height="195" ctype="ImageViewObjectData">
                <Size X="320.0000" Y="960.0000" />
                <AnchorPoint />
                <Position X="320.0000" />
                <Scale ScaleX="1.0000" ScaleY="1.0000" />
                <CColor A="255" R="255" G="255" B="255" />
                <PrePosition X="0.5000" />
                <PreSize X="0.5000" Y="1.0000" />
                <FileData Type="Normal" Path="Common/BG_Battle_2.png" Plist="" />
              </AbstractNodeData>
            </Children>
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="320.0000" Y="480.0000" />
            <Scale ScaleX="1.0000" ScaleY="1.0000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.5000" />
            <PreSize X="1.0000" Y="1.0000" />
            <FileData Type="Normal" Path="Common/BG_Battle.png" Plist="" />
          </AbstractNodeData>
          <AbstractNodeData Name="Image_Tournaments_BG" ActionTag="-494358391" VisibleForFrame="False" Alpha="0" Tag="2607" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" TopEage="187" BottomEage="187" Scale9OriginY="187" Scale9Width="1" Scale9Height="195" ctype="ImageViewObjectData">
            <Size X="640.0000" Y="960.0000" />
            <Children>
              <AbstractNodeData Name="Image_Left" ActionTag="-459107939" Tag="189" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" RightMargin="320.0000" LeftEage="52" RightEage="52" TopEage="187" BottomEage="187" Scale9OriginX="52" Scale9OriginY="187" Scale9Width="56" Scale9Height="195" ctype="ImageViewObjectData">
                <Size X="320.0000" Y="960.0000" />
                <AnchorPoint ScaleX="1.0000" />
                <Position X="320.0000" />
                <Scale ScaleX="1.0000" ScaleY="1.0000" />
                <CColor A="255" R="255" G="255" B="255" />
                <PrePosition X="0.5000" />
                <PreSize X="0.5000" Y="1.0000" />
                <FileData Type="Normal" Path="Common/BG_Tournament_2.png" Plist="" />
              </AbstractNodeData>
              <AbstractNodeData Name="Image_Right" ActionTag="-1882984731" Tag="190" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" LeftMargin="320.0000" LeftEage="52" RightEage="52" TopEage="187" BottomEage="187" Scale9OriginX="52" Scale9OriginY="187" Scale9Width="56" Scale9Height="195" ctype="ImageViewObjectData">
                <Size X="320.0000" Y="960.0000" />
                <AnchorPoint />
                <Position X="320.0000" />
                <Scale ScaleX="1.0000" ScaleY="1.0000" />
                <CColor A="255" R="255" G="255" B="255" />
                <PrePosition X="0.5000" />
                <PreSize X="0.5000" Y="1.0000" />
                <FileData Type="Normal" Path="Common/BG_Tournament_2.png" Plist="" />
              </AbstractNodeData>
            </Children>
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="320.0000" Y="480.0000" />
            <Scale ScaleX="1.0000" ScaleY="1.0000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.5000" />
            <PreSize X="1.0000" Y="1.0000" />
            <FileData Type="Normal" Path="Common/BG_Tournament.png" Plist="" />
          </AbstractNodeData>
          <AbstractNodeData Name="Image_Retro_BG" ActionTag="312679254" VisibleForFrame="False" Alpha="0" Tag="5827" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" TopEage="15" BottomEage="15" Scale9OriginY="15" Scale9Width="1" Scale9Height="539" ctype="ImageViewObjectData">
            <Size X="640.0000" Y="960.0000" />
            <Children>
              <AbstractNodeData Name="Image_Left" ActionTag="-38770047" Tag="191" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" RightMargin="320.0000" LeftEage="52" RightEage="52" TopEage="187" BottomEage="187" Scale9OriginX="52" Scale9OriginY="187" Scale9Width="56" Scale9Height="195" ctype="ImageViewObjectData">
                <Size X="320.0000" Y="960.0000" />
                <AnchorPoint ScaleX="1.0000" />
                <Position X="320.0000" />
                <Scale ScaleX="1.0000" ScaleY="1.0000" />
                <CColor A="255" R="255" G="255" B="255" />
                <PrePosition X="0.5000" />
                <PreSize X="0.5000" Y="1.0000" />
                <FileData Type="Normal" Path="Common/BG_Retro_2.png" Plist="" />
              </AbstractNodeData>
              <AbstractNodeData Name="Image_Right" ActionTag="-788426156" Tag="192" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" PercentWidthEnable="True" PercentHeightEnable="True" PercentWidthEnabled="True" PercentHeightEnabled="True" LeftMargin="320.0000" LeftEage="52" RightEage="52" TopEage="187" BottomEage="187" Scale9OriginX="52" Scale9OriginY="187" Scale9Width="56" Scale9Height="195" ctype="ImageViewObjectData">
                <Size X="320.0000" Y="960.0000" />
                <AnchorPoint />
                <Position X="320.0000" />
                <Scale ScaleX="1.0000" ScaleY="1.0000" />
                <CColor A="255" R="255" G="255" B="255" />
                <PrePosition X="0.5000" />
                <PreSize X="0.5000" Y="1.0000" />
                <FileData Type="Normal" Path="Common/BG_Retro_2.png" Plist="" />
              </AbstractNodeData>
            </Children>
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="320.0000" Y="480.0000" />
            <Scale ScaleX="1.0000" ScaleY="1.0000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.5000" />
            <PreSize X="1.0000" Y="1.0000" />
            <FileData Type="Normal" Path="Common/BG_SinglePlayer.png" Plist="" />
          </AbstractNodeData>
        </Children>
      </ObjectData>
    </Content>
  </Content>
</GameFile>