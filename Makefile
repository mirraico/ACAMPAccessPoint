.PHONY: clean All

All:
	@echo "----------Building project:[ ACAMPAccessPoint - Debug ]----------"
	@"$(MAKE)" -f  "ACAMPAccessPoint.mk"
clean:
	@echo "----------Cleaning project:[ ACAMPAccessPoint - Debug ]----------"
	@"$(MAKE)" -f  "ACAMPAccessPoint.mk" clean
