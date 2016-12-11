CXX := g++
FLAGS := -std=c++11 -Wall -pthread
INCLUDE := include
BUILD := build
SRC := src
TEST := test
BIN := bin

all: $(BIN)/test_dv_distributed $(BIN)/test_ls_distrubuted $(BIN)/test_ls_centralized_center $(BIN)/test_ls_centralized_client

$(BIN)/test_ls_centralized_center: $(BUILD)/test_ls_centralized_center.o $(BUILD)/udp.o $(BUILD)/timeout.o $(BUILD)/ls.o $(BUILD)/ls_centralized_center.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I $(INCLUDE) -o $@ $^

$(BIN)/test_ls_centralized_client: $(BUILD)/test_ls_centralized_client.o $(BUILD)/udp.o $(BUILD)/timeout.o $(BUILD)/ls.o $(BUILD)/ls_centralized_client.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I $(INCLUDE) -o $@ $^

$(BIN)/test_ls_distrubuted: $(BUILD)/test_ls_distributed.o $(BUILD)/udp.o $(BUILD)/timeout.o $(BUILD)/ls.o $(BUILD)/ls_distributed.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I $(INCLUDE) -o $@ $^

$(BIN)/test_dv_distributed: $(BUILD)/test_dv_distributed.o $(BUILD)/udp.o $(BUILD)/timeout.o $(BUILD)/dv.o $(BUILD)/dv_distributed.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I $(INCLUDE) -o $@ $^

$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p ./$(BUILD)
	$(CXX) $(FLAGS) -I $(INCLUDE) -c -o $@ $<


clean:
	rm -rfv $(BIN) $(BUILD)
