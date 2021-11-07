#ifndef PROJECT_INCLUDE
#define PROJECT_INCLUDE

#define DEFAULT_PROJECT_NAME "NewProject.MFIT"

class Project
{
	private:

		Video *vdo;
		Frame *frame;
		char path_cy[256];
		char filename_cy[256];

		int userCutThreshold; // Este limiar será em %, o que representa a % da altura do frame.
		int userMinCanny;
		int userMaxCanny;

		void clearTransitionList();
		void clearEffectList();

	public:

		Project();
		~Project();

		int openProject(QString);
		int saveProject(QString);
		int openVideo(QString);

		std::vector<Transition> transitionList;  // Transicoes no video
		std::vector<Effect*>    effectList;      // Efeitos no video

		Video *getVideo();
		Frame *getFrame();
		char  *getPath();
		char  *getName();

		void setUserThreshold(int threshold);
		int getUserThreshold();

		void setUserMinCanny(int userMinCanny);
		int getUserMinCanny();

		void setUserMaxCanny(int userMaxCanny);
		int getUserMaxCanny();

		void sortTransitionList(void);

		long FrameToTimelinePos(long frame);
		long TimelinePosToFrame(long pos);

		void renderVideo(char *filename_cy);

		Frame* applyEffect(Frame *frame, long pos);

		int getTransitionByPos(int pos_x);

		void removeEffect(int ind);
		void removeTransition(int ind);

};
#endif
