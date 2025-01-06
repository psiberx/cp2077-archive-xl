public abstract native class ArchiveXL {
    public static native func GetBodyType(puppet: wref<GameObject>) -> CName
    public static native func EnableGarmentOffsets()
    public static native func DisableGarmentOffsets()
    public static native func EnablePhotoModeAlternativeControls()
    public static native func DisablePhotoModeAlternativeControls()
    public static native func Require(version: String) -> Bool
    public static native func Version() -> String
}
