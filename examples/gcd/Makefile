TARGET=gcd
OBJDIR=obj
TXTDIR=txt

SRC=$(wildcard *.s)
OBJ=$(patsubst %.s, $(OBJDIR)/%.o, $(SRC))

$(TARGET): $(OBJDIR) $(TXTDIR) $(OBJ)
	lnk -o $(TARGET) -t $(TXTDIR)/$(TARGET).txt $(OBJ)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TXTDIR):
	mkdir -p $(TXTDIR)

$(OBJDIR)/%.o: %.s
	ass -o $@ -t $(TXTDIR)/$<.txt $<

clean:
	rm -rf $(OBJDIR)/*.o $(TXTDIR)/*.txt *.log $(TARGET)

.PHONY: clean

