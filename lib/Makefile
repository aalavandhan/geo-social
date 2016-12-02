PROG = NNTopkUsers
CC = g++-4.8 -std=c++0x
CPPFLAGS = -g -O2 -Wall -fpermissive
LDFLAGS = -I/usr/local/include
LFLAGS = -L/usr/local/lib
#LIBS = -lmongoclient -lboost_thread -lboost_filesystem -lboost_program_options -lboost_system
OBJS = Group.o Utilities.o GPOs.o Cell.o Point.o Grid.o QuadTreeNode.o IncrVisitor.o SPOs.o Visitor.o Value.o User.o Pair.o FileIndex.o murmur.o dablooms.o InvertedIndexGen.o BasicGSQueries.o topkUsers.o topkGSK.o mGroup.o NNTopkUsers.o
#SRCS = complexQueries/topkUsers.cpp basicGSQueries/BasicGSQueries.cpp SPOs/MemoryMapWeighted/Pair.cpp SPOs/MemoryMapWeighted/User.cpp SPOs/MemoryMapWeighted/Graph.cpp SPOs/MemoryMapWeighted/Graph_D.cpp SPOs/MemoryMap/Value.cpp SPOs/MemoryMap/SPOs.cpp SPOs/MemoryMap/SPOs_D.cpp GPOs/MemoryGrid/grid/Visitor.cpp GPOs/MemoryGrid/grid/IncrVisitor.cpp GPOs/MemoryGrid/grid/Point.cpp GPOs/MemoryGrid/grid/Grid.cpp GPOs/MemoryGrid/grid/Cell.cpp GPOs/MemoryGrid/GPOs.cpp GPOs/MemoryGrid/GPOs_D.cpp utilities/Utilities.cpp utilities/Group.cpp testNNTopkUsers.cpp

$(PROG) : $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS) $(LFLAGS) $(LIBS)

-include $(OBJS:.o=.d)

NNTopkUsers.o : NNTopkUsers.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c NNTopkUsers.cpp
	$(CC) -MM NNTopkUsers.cpp > NNTopkUsers.d
Group.o : utilities/Group.cpp headers.h
	$(CC) $(CPPFLAGS) -c utilities/Group.cpp
	$(CC) -MM -c utilities/Group.cpp > Group.d
Utilities.o : utilities/Utilities.cpp headers.h
	$(CC) $(CPPFLAGS) -c utilities/Utilities.cpp
	$(CC) -MM utilities/Utilities.cpp > Utilities.d
GPOs.o : GPOs/MemoryGrid/GPOs.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c GPOs/MemoryGrid/GPOs.cpp
	$(CC) -MM GPOs/MemoryGrid/GPOs.cpp > GPOs.d
#GPOs_D.o : GPOs/MemoryGrid/GPOs_D.cpp headersDecentralized.h
#	$(CC) $(CPPFLAGS) -c GPOs/MemoryGrid/GPOs_D.cpp
#	$(CC) -MM GPOs/MemoryGrid/GPOs_D.cpp > GPOs_D.d
#GPOsMongo.o : GPOs/MongoDbGrid/GPOsMongo.cpp
#	$(CC) $(CPPFLAGS) -c GPOs/MongoDbGrid/GPOsMongo.cpp
#	$(CC) -MM GPOs/MongoDbGrid/GPOsMongo.cpp > GPOsMongo.d
InvertedIndexGen.o : TPOs/InvertedIndexGen.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c  TPOs/InvertedIndexGen.cpp
	$(CC) -MM  TPOs/InvertedIndexGen.cpp > InvertedIndexGen.d
FileIndex.o : TPOs/FileIndex.cpp headers.h
	$(CC) $(CPPFLAGS) -c TPOs/FileIndex.cpp
	$(CC) -MM TPOs/FileIndex.cpp > FileIndex.d
murmur.o : bloom/murmur.c bloom/murmur.h
	$(CC) $(CPPFLAGS) -c bloom/murmur.c
	$(CC) -MM bloom/murmur.c > murmur.d
dablooms.o : bloom/dablooms.c headersMemory.h
	$(CC) $(CPPFLAGS) -c bloom/dablooms.c
	$(CC) -MM bloom/dablooms.c > dablooms.d
Cell.o : GPOs/MemoryGrid/grid/Cell.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c GPOs/MemoryGrid/grid/Cell.cpp
	$(CC) -MM GPOs/MemoryGrid/grid/Cell.cpp > Cell.d
QuadTreeNode.o : GPOs/MemoryGrid/grid/QuadTreeNode.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c GPOs/MemoryGrid/grid/QuadTreeNode.cpp
	$(CC) -MM GPOs/MemoryGrid/grid/QuadTreeNode.cpp > QuadTreeNode.d
Grid.o : GPOs/MemoryGrid/grid/Grid.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c GPOs/MemoryGrid/grid/Grid.cpp
	$(CC) -MM GPOs/MemoryGrid/grid/Grid.cpp > Grid.d
Point.o : GPOs/MemoryGrid/grid/Point.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c GPOs/MemoryGrid/grid/Point.cpp
	$(CC) -MM GPOs/MemoryGrid/grid/Point.cpp > Point.d
IncrVisitor.o : GPOs/MemoryGrid/grid/IncrVisitor.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c GPOs/MemoryGrid/grid/IncrVisitor.cpp
	$(CC) -MM GPOs/MemoryGrid/grid/IncrVisitor.cpp > IncrVisitor.d
Visitor.o : GPOs/MemoryGrid/grid/Visitor.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c GPOs/MemoryGrid/grid/Visitor.cpp
	$(CC) -MM GPOs/MemoryGrid/grid/Visitor.cpp > Visitor.d
SPOs.o : SPOs/MemoryMap/SPOs.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c SPOs/MemoryMap/SPOs.cpp
	$(CC) -MM SPOs/MemoryMap/SPOs.cpp > SPOs.d
#SPOs_D.o : SPOs/MemoryMap/SPOs_D.cpp headersDecentralized.h
#	$(CC) $(CPPFLAGS) -c SPOs/MemoryMap/SPOs_D.cpp
#	$(CC) -MM SPOs/MemoryMap/SPOs_D.cpp > SPOs_D.d
Value.o : SPOs/MemoryMap/Value.cpp  headersMemory.h
	$(CC) $(CPPFLAGS) -c SPOs/MemoryMap/Value.cpp
	$(CC) -MM SPOs/MemoryMap/Value.cpp > Value.d
User.o : SPOs/MemoryMapWeighted/User.cpp  headersMemory.h
	$(CC) $(CPPFLAGS) -c SPOs/MemoryMapWeighted/User.cpp
	$(CC) -MM SPOs/MemoryMapWeighted/User.cpp > User.d
Pair.o : SPOs/MemoryMapWeighted/Pair.cpp  headersMemory.h
	$(CC) $(CPPFLAGS) -c SPOs/MemoryMapWeighted/Pair.cpp
	$(CC) -MM SPOs/MemoryMapWeighted/Pair.cpp > Pair.d
#Graph.o : SPOs/MemoryMapWeighted/Graph.cpp headersMemory.h
#	$(CC) $(CPPFLAGS) -c SPOs/MemoryMapWeighted/Graph.cpp
#	$(CC) -MM SPOs/MemoryMapWeighted/Graph.cpp > Graph.d
#Graph_D.o : SPOs/MemoryMapWeighted/Graph_D.cpp headersDecentralized.h
#	$(CC) $(CPPFLAGS) -c SPOs/MemoryMapWeighted/Graph_D.cpp
#	$(CC) -MM SPOs/MemoryMapWeighted/Graph_D.cpp > Graph_D.d
#SPOsMongo.o : SPOs/MongoDbMap/SPOsMongo.cpp
#	$(CC) $(CPPFLAGS) -c SPOs/MongoDbMap/SPOsMongo.cpp
#	$(CC) -MM SPOs/MongoDbMap/SPOsMongo.cpp > SPOsMongo.d
BasicGSQueries.o : basicGSQueries/BasicGSQueries.cpp headers.h
	$(CC) $(CPPFLAGS) -c basicGSQueries/BasicGSQueries.cpp
	$(CC) -MM basicGSQueries/BasicGSQueries.cpp > BasicGSQueries.d
topkUsers.o : complexQueries/topkUsers.cpp headers.h
	$(CC) $(CPPFLAGS) -c complexQueries/topkUsers.cpp
	$(CC) -MM complexQueries/topkUsers.cpp > topkUsers.d
	$(CC) -MM basicGSQueries/BasicGSQueries.cpp > BasicGSQueries.d
topkGSK.o : GSKqueries/topkGSK.cpp headersMemory.h
	$(CC) $(CPPFLAGS) -c GSKqueries/topkGSK.cpp
	$(CC) -MM GSKqueries/topkGSK.cpp > topkGSK.d
mGroup.o : complexQueries/mGroup.cpp headers.h
	$(CC) $(CPPFLAGS) -c complexQueries/mGroup.cpp
	$(CC) -MM complexQueries/mGroup.cpp > .d

.PHONY : clean
clean:
	rm -f core $(OBJS) *.d

.PHONY: cleanest
cleanest:
	rm -f core $(PROG) $(OBJS) *.d
