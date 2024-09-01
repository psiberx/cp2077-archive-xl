@wrapMethod(ScannervehicleGameController)
protected cb func OnVehicleManufacturerChanged(value: Variant) -> Bool {
    wrappedMethod(value);
    if this.m_isValidVehicleManufacturer {
        let vehicleManufacturer = FromVariant<ref<ScannerVehicleManufacturer>>(value);
        let iconRecord = TweakDBInterface.GetUIIconRecord(TDBID.Create("UIIcon." + vehicleManufacturer.GetVehicleManufacturer()));
        inkImageRef.SetAtlasResource(this.m_vehicleManufacturer, iconRecord.AtlasResourcePath());
    }
}