@if(ModuleExists("Codeware"))
@wrapMethod(characterCreationBodyMorphMenu)
protected cb func OnInitialize() -> Bool {
    wrappedMethod();

    let pickerWidget = inkWidgetRef.Get(this.m_colorPicker) as inkCompoundWidget;
    let pickerController = inkWidgetRef.GetController(this.m_colorPicker) as characterCreationBodyMorphOptionColorPicker;

    pickerWidget.GetWidget(n"fluff_1").SetVisible(false);
    pickerWidget.GetWidget(n"fluff_3").SetVisible(false);

    let gridWidget = inkWidgetRef.Get(pickerController.m_grid) as inkCompoundWidget;
    gridWidget.SetChildMargin(new inkMargin(10.0, 0.0, 0.0, 12.0));

    let containerWidget = gridWidget.parentWidget as inkCompoundWidget;
    containerWidget.SetHAlign(inkEHorizontalAlign.Fill);
    containerWidget.SetMargin(new inkMargin(0.0, 0.0, 0.0, 0.0));
    containerWidget.SetPadding(new inkMargin(0.0, 0.0, 0.0, 0.0));

    let scrollWrapper = new inkCanvas();
    scrollWrapper.SetName(n"scrollWrapper");
    scrollWrapper.SetHAlign(inkEHorizontalAlign.Fill);
    scrollWrapper.SetVAlign(inkEVerticalAlign.Top);
    scrollWrapper.SetMargin(new inkMargin(0.0, 12.0, 0.0, 12.0));
    scrollWrapper.SetHeight((160.0 + 10.0) * 7.5);
    scrollWrapper.SetInteractive(true);

    let scrollArea = new inkScrollArea();
    scrollArea.SetName(n"scrollArea");
    scrollArea.SetAnchor(inkEAnchor.Fill);
    scrollArea.SetMargin(new inkMargin(0, 0, 16.0, 0));
    scrollArea.fitToContentDirection = inkFitToContentDirection.Horizontal;
    scrollArea.useInternalMask = true;
    scrollArea.Reparent(scrollWrapper, -1);

    let sliderArea = new inkCanvas();
    sliderArea.SetName(n"sliderArea");
    sliderArea.SetAnchor(inkEAnchor.RightFillVerticaly);
    sliderArea.SetSize(new Vector2(15.0, 0.0));
    sliderArea.SetMargin(new inkMargin(0.0, 2.0, 30.0, 25.0));
    sliderArea.SetInteractive(true);
    sliderArea.Reparent(scrollWrapper, -1);

    let sliderFill = new inkRectangle();
    sliderFill.SetName(n"sliderFill");
    sliderFill.SetAnchor(inkEAnchor.Fill);
    sliderFill.SetOpacity(0.05);
    sliderFill.Reparent(sliderArea, -1);

    let sliderHandle = new inkRectangle();
    sliderHandle.SetName(n"sliderHandle");
    sliderHandle.SetAnchor(inkEAnchor.TopFillHorizontaly);
    sliderHandle.SetSize(new Vector2(15.0, 40.0));
    sliderHandle.SetInteractive(true);
    sliderHandle.Reparent(sliderArea, -1);

    let sliderController = new inkSliderController();
    sliderController.slidingAreaRef = inkWidgetRef.Create(sliderArea);
    sliderController.handleRef = inkWidgetRef.Create(sliderHandle);
    sliderController.direction = inkESliderDirection.Vertical;
    sliderController.autoSizeHandle = true;
    sliderController.percentHandleSize = 0.4;
    sliderController.minHandleSize = 40.0;
    sliderController.Setup(0, 1, 0, 0);

    let scrollController = new inkScrollController();
    scrollController.ScrollArea = inkScrollAreaRef.Create(scrollArea);
    scrollController.VerticalScrollBarRef = inkWidgetRef.Create(sliderArea);
    scrollController.autoHideVertical = true;

    scrollWrapper.Reparent(containerWidget);
    gridWidget.Reparent(scrollArea);

    // Binding properties after adding widget to a tree,
    // so style resource can be inherited from parent
    sliderFill.BindProperty(n"tintColor", n"MainColors.Red");
    sliderHandle.BindProperty(n"tintColor", n"MainColors.Red");

    // Attaching controllers after adding widget to a tree,
    // so they can be properly initialized
    sliderArea.AttachController(sliderController);
    scrollWrapper.AttachController(scrollController);
}
