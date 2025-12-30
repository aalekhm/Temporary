<GameFile>
  <PropertyGroup Name="CSBSceneMainMenu" Type="Scene" ID="07788f7a-dd1b-4931-9b67-620b97b577e3" Version="3.10.0.0" />
  <Content ctype="GameProjectContent">
    <Content>
      <Animation Duration="60" Speed="1.0000" ActivedAnimationName="animationIn">
        <Timeline ActionTag="1447176059" Property="Alpha">
          <IntFrame FrameIndex="0" Value="0">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="30" Value="255">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="31" Value="255">
            <EasingData Type="0" />
          </IntFrame>
          <IntFrame FrameIndex="60" Value="0">
            <EasingData Type="0" />
          </IntFrame>
        </Timeline>
        <Timeline ActionTag="1386545511" Property="VisibleForFrame">
          <BoolFrame FrameIndex="0" Tween="False" Value="False" />
          <BoolFrame FrameIndex="30" Tween="False" Value="True" />
          <BoolFrame FrameIndex="31" Tween="False" Value="False" />
        </Timeline>
      </Animation>
      <AnimationList>
        <AnimationInfo Name="animationIn" StartIndex="0" EndIndex="30">
          <RenderColor A="255" R="255" G="127" B="80" />
        </AnimationInfo>
        <AnimationInfo Name="animationOut" StartIndex="31" EndIndex="60">
          <RenderColor A="255" R="210" G="180" B="140" />
        </AnimationInfo>
      </AnimationList>
      <ObjectData Name="CSBSceneMainMenu" CustomClassName="CSBSceneMainMenu" Tag="1447" ctype="GameNodeObjectData">
        <Size X="640.0000" Y="960.0000" />
        <Children>
          <AbstractNodeData Name="ImageView_Dummy" ActionTag="1447176059" Alpha="0" Tag="1448" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" LeftMargin="265.0000" RightMargin="265.0000" TopMargin="425.0000" BottomMargin="425.0000" LeftEage="36" RightEage="36" TopEage="36" BottomEage="36" Scale9OriginX="36" Scale9OriginY="36" Scale9Width="38" Scale9Height="38" ctype="ImageViewObjectData">
            <Size X="110.0000" Y="110.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="320.0000" Y="480.0000" />
            <Scale ScaleX="1.0000" ScaleY="1.0000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.5000" />
            <PreSize X="0.1719" Y="0.1146" />
            <FileData Type="Normal" Path="Common/ActivityIcon_Activity.png" Plist="" />
          </AbstractNodeData>
          <AbstractNodeData Name="Button_Next" ActionTag="1386545511" VisibleForFrame="False" CallBackType="Touch" CallBackName="OnTouch" Tag="1433" IconVisible="False" PositionPercentXEnabled="True" PositionPercentYEnabled="True" LeftMargin="437.0000" RightMargin="53.0000" TopMargin="837.5000" BottomMargin="69.5000" TouchEnable="True" FontSize="24" ButtonText="Next" Scale9Enable="True" LeftEage="15" RightEage="15" TopEage="11" BottomEage="11" Scale9OriginX="15" Scale9OriginY="11" Scale9Width="23" Scale9Height="31" ShadowOffsetX="2.0000" ShadowOffsetY="-2.0000" ctype="ButtonObjectData">
            <Size X="150.0000" Y="53.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="512.0000" Y="96.0000" />
            <Scale ScaleX="1.0000" ScaleY="1.0000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.8000" Y="0.1000" />
            <PreSize X="0.2344" Y="0.0552" />
            <FontResource Type="Normal" Path="TradeGothicNextLTPro-Rg.ttf" Plist="" />
            <TextColor A="255" R="0" G="0" B="0" />
            <DisabledFileData Type="Normal" Path="Common/Button_GreySmall.png" Plist="" />
            <PressedFileData Type="Normal" Path="Common/Button_GreySmall.png" Plist="" />
            <NormalFileData Type="Normal" Path="Common/Button_Blue_Small.png" Plist="" />
            <OutlineColor A="255" R="255" G="0" B="0" />
            <ShadowColor A="255" R="110" G="110" B="110" />
          </AbstractNodeData>
        </Children>
      </ObjectData>
    </Content>
  </Content>
</GameFile>