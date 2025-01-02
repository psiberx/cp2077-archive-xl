@wrapMethod(PhotoModePlayerEntityComponent)
private final func EquipWeaponOfThisType(typesList: array<gamedataItemType>) -> Void {
    this.ListAllCurrentItems();

    wrappedMethod(typesList);
}

@wrapMethod(PhotoModePlayerEntityComponent)
private final func ListAllCurrentItems() {
    ArrayClear(this.availableCurrentItemsList);
    this.TS.GetItemList(this.fakePuppet, this.availableCurrentItemsList);

    wrappedMethod();
}
