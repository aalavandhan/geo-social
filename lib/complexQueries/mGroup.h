class mGroup{

	private:
		IGPOs *gpos;
		ISPOs *spos;
		SimpleQueries *sq;
		Utilities util;
		int usersChecked;
		double current_dist;
		int m_group;

	public:
		mGroup(IGPOs *spatialIndex, ISPOs *socialGraph, SimpleQueries *simpleQueries);
		~mGroup();
		double computeLBTime;
		double computeGroupLB(res_point* user, vector<res_point*>* seen_users, int m, char* f);
		double computeGroupLBNew(vector<Group*>* seen_groups, int m);


		priority_queue <Group*, vector<Group*>, Group::descending >* EAGER(double x, double y, int m, int k, int incrStep, char* f);
		priority_queue <Group*, vector<Group*>, Group::descending >* LAZY(double x, double y, int m, int k, int incrStep, char* f);
		priority_queue <Group*, vector<Group*>, Group::descending >* EAGER_OPT(double x, double y, int m, int k, int incrStep);

		priority_queue <Group*, vector<Group*>, Group::descending >* EAGER_OPT_RANGE(double x, double y, int m, int k, int incrStep);
		priority_queue <Group*, vector<Group*>, Group::descending >* EAGER_OPT_FINAL(double x, double y, int m, int k, int incrStep);
		//priority_queue <Group*, vector<Group*>, Group::descending >* tight(double x, double y, int m, int k, int incrStep);
		int getNumOfUsersChecked();


};

