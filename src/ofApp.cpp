#include "ofApp.h"

ps_decoder_t *ps;
cmd_ln_t *config;
std::vector<new_utterance> result;
std::vector<std::string> files;

std::vector<std::string> filename_split;
std::string directoryString = "/Users/benjgorman/Desktop/CUNY/old_cleaned/converted_raw/";
//std::string directoryString = "/Users/benjgorman/Desktop/CUNY/old_cleaned/cleaned/converted_raw/";


class CMUword
{
public:
    
    std::string full_string="";
    
    std::string word="";
    
    std::vector<string> phonemes;
    

    friend bool operator== ( const CMUword &n1, const CMUword &n2);
};

vector<CMUword> CMUdict; //declare a vector of strings to store data

//--------------------------------------------------------------
void ofApp::setup()
{
    
    ifstream fin; //declare a file stream
    fin.open( ofToDataPath("/Users/benjgorman/desktop/en-us/cmudict-en-us.dict").c_str() ); //open your text file
    
    std::vector<std::string> phonemes;
    
    while(fin!=NULL) //as long as theres still text to be read
    {
        string str; //declare a string for storage
        getline(fin, str); //get a line from the file, put it in the string
        
        CMUword temp;
        
        
        std::cout << "word is" + str << std::endl;
        
        phonemes.clear();
        
        std::string delimiter = " ";
        
        size_t pos = 0;
        std::string token;
        while ((pos = str.find(delimiter)) != std::string::npos)
        {
            token = str.substr(0, pos);
            phonemes.push_back(token);
            str.erase(0, pos + delimiter.length());
        }
        phonemes.push_back(str); //add the last token
        temp.word = phonemes[0];
        
        temp.phonemes = phonemes;
        
        
        CMUdict.push_back(temp); //push the string onto a vector of strings
        
    }
    
    
    
    //POCKETSPHINX SETUP STUFF HERE
    
    // need to read through the files and built a list of them here
    
    //some path, may be absolute or relative to bin/data
    ofDirectory dir(directoryString);
    //only show png files
    dir.allowExt("raw");
    //populate the directory object
    dir.listDir();
    
    //go through and print out all the paths
    for(int i = 0; i < dir.numFiles(); i++)
    {
        files.push_back(dir.getName(i));
       
    }
    
    
    
    int j =0;
    while (j<files.size())
    {
        bEngineInitialed = engineInit();
        filename_split.clear();

        std::string delimiter = ".";
        
        std::string filename = files[j];
        
        size_t pos = 0;
        std::string token;
        while ((pos = filename.find(delimiter)) != std::string::npos) {
            token = filename.substr(0, pos);
            std::cout << token << std::endl;
            filename_split.push_back(token);
            filename.erase(0, pos + delimiter.length());
        }
        
        
        engineOpen(files[j]);
        j++;
    }
    
}

bool operator== ( const CMUword &n1, const CMUword &n2)
{
    return n1.word == n2.word;
}

//--------------------------------------------------------------
void ofApp::update()
{
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    
}


//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels)
{
    
}


void ofApp::process_result()
{
    int frame_rate = cmd_ln_int32_r(config, "-frate");
    ps_seg_t *iter = ps_seg_iter(ps, NULL);
    printf("\n\n");
    
    //this will save our output as an ofxTimelineInput xml settings file
    ofxXmlSettings keyframes;
    keyframes.addTag("keyframes");
    keyframes.pushTag("keyframes");
    
    int i=0;
    
    while (iter != NULL)
    {
        int32 sf, ef, pprob;
        float conf;
        
        ps_seg_frames(iter, &sf, &ef);
        pprob = ps_seg_prob(iter, NULL, NULL, NULL);
        conf = logmath_exp(ps_get_logmath(ps), pprob);
        
        //here is where we process the word
        
        new_utterance new_utt;
        new_utt.conf = conf;
        new_utt.sf = sf;
        new_utt.st = (float)sf / frame_rate;
        new_utt.ef = ef;
        new_utt.et = (float) ef / frame_rate;
        new_utt.utt = ps_seg_word(iter);
        
        printf("Recognised: %s %.3f %.3f %f\n", ps_seg_word(iter), new_utt.st,
               new_utt.et, new_utt.conf);
        
        //each position tag represents one point
        
        //so set the three values in the file
        
        
        
        
        
        //here we should do a check and see if there are brackets in the file, ( > or [ and split it up as needed. if it's a ( we want to split it up, if it's a [] we should keep it and if it's a < we can ignore it.
        
        std::string word = ps_seg_word(iter);
        
        //search in here before we take out the info, we need to make sure we have the varient
        
        CMUword caption;
        
        caption.word = word;
        
        std::vector<CMUword>::iterator myCMUVectorIterator;
        
        std::string initial_phoneme;
        
        
        if ((myCMUVectorIterator = std::find(CMUdict.begin(), CMUdict.end(), caption)) != CMUdict.end())
        {
            
            initial_phoneme = myCMUVectorIterator->phonemes[1].c_str();
            cout << "The initial phoneme is: " + initial_phoneme;
        }
        
        std::string token;
        
        bool skip_word = false;
        
        std::size_t found=word.find('(');
        if (found!=std::string::npos)
        {
            std::string delimiter = "(";
            
            
            size_t pos = 0;
            
            while ((pos = word.find(delimiter)) != std::string::npos) {
                token = word.substr(0, pos);
                filename_split.push_back(token);
                word.erase(0, pos + delimiter.length());
            }

        }
        
        std::size_t found_angle=word.find('<');
        if (found_angle!=std::string::npos)
        {
            skip_word = true;
        }
        
        std::size_t found_square=word.find('[');
        if (found_square!=std::string::npos)
        {
            
        }
        
        
        
        //removes any pronounciation variants from our XML files
        
        ofxTimecode timecode;
        //if our token is not empty then we'll use word
            if (!token.empty())
            {
                keyframes.addTag("key");
                keyframes.pushTag("key",i);
                
                keyframes.addValue("flag", token);
                
                keyframes.addValue("time", timecode.timecodeForSeconds(new_utt.st).c_str());
                keyframes.addValue("endtime", timecode.timecodeForSeconds(new_utt.et).c_str());
                keyframes.addValue("value", 1);
                
                
                int j = 1; //skip the word
                
                if (myCMUVectorIterator->phonemes.size() >1)
                {
                    keyframes.addTag("phonemic_translation");
                    keyframes.pushTag("phonemic_translation");
                    while (j<myCMUVectorIterator->phonemes.size())
                    {
                        
                        keyframes.addValue("phoneme", myCMUVectorIterator->phonemes[j].c_str());
                        j++;
                    }

                    keyframes.popTag();//pop position
                }
                
                keyframes.popTag();//pop position
                
                i++;
            }
            else if (skip_word == false)
            {
                keyframes.addTag("key");
                keyframes.pushTag("key",i);

                keyframes.addValue("flag", word);
                
                keyframes.addValue("time", timecode.timecodeForSeconds(new_utt.st).c_str());
                keyframes.addValue("endtime", timecode.timecodeForSeconds(new_utt.et).c_str());
                keyframes.addValue("value", 1);
                
                int j = 1; //skip the word
                
                if (myCMUVectorIterator->phonemes.size() >1)
                {
                    keyframes.addTag("phonemic_translation");
                    keyframes.pushTag("phonemic_translation");
                    while (j<myCMUVectorIterator->phonemes.size())
                    {
                        
                        keyframes.addValue("phoneme", myCMUVectorIterator->phonemes[j].c_str());
                        j++;
                    }

                    
                    keyframes.popTag();//pop position
                }
                keyframes.popTag();//pop position
                
                i++;
            }
        
        //keyframes.popTag();
        
        result.push_back(new_utt);
        
        iter = ps_seg_next(iter);
        
        
    }
    
    keyframes.popTag(); //pop position
    keyframes.saveFile((filename_split[0] + ".xml").c_str());
    
    printf("\n\n");
    
    engineExit();

}


bool ofApp::engineInit()
{
//words
   config = cmd_ln_init(NULL, ps_args(), TRUE, "-hmm", MODELDIR "/en-us/en-us", "-lm", MODELDIR "/en-us/en-us.lm.dmp", "-dict", MODELDIR "/en-us/cmudict-en-us.dict", NULL);
    
    

    //config = cmd_ln_init(NULL, ps_args(), TRUE, "-hmm", MODELDIR "/en-us/en-us", "-allphone", MODELDIR "/en-us/en-us-phone.lm.dmp", "-backtrace", "yes", "-beam", "1e-20", "-pbeam", "1e-20", "-lw", "2.0", NULL);
    
    
    
    if (config == NULL)
    {
        return false;
    }
    ps = ps_init(config);
    
    if (ps == NULL)
    {
        return false;
    }
}

int ofApp::engineExit()
{
    
    //fclose(fh);
    ps_free(ps);
    cmd_ln_free_r(config);
    return 0;
    
}

int ofApp::engineOpen(string filename)
{
    
    FILE *fh;
    char const *hyp, *uttid;
    int16 buf[512];
    int rv; int32 score;

    fh = fopen((directoryString + filename).c_str(), "rb");
    if (fh == NULL)
    {
        return -1;
    }
    rv = ps_start_utt(ps);
    if (rv < 0) return 1;
    while (!feof(fh))
    {
        size_t nsamp; nsamp = fread(buf, 2, 512, fh);
        rv = ps_process_raw(ps, buf, nsamp, FALSE, FALSE);
    }
    
    engineClose();

    
}

int ofApp::engineClose()
{
    char const *hyp, *uttid;
    int rv;
    int32 score;
    
    rv = ps_end_utt(ps);
    if (rv < 0)
    {
        return 1;
    }
    hyp = ps_get_hyp(ps, &score);
    if (hyp == NULL)
    {
        return 1;
    }
    
    printf("NEWLINE_________\n\n\nRecognized: %s\n", hyp);
    process_result();

    
}

int ofApp::engineSentAudio(short *audioBuf, int audioSize)
{
    
}

bool ofApp::isEngineOpened()
{
    
}

char * ofApp::engineGetText()
{

}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key)
{
    if( key == 's' )
    {
        soundStream.start();
        ps_start_utt(ps);
    }
    
    if( key == 'e' ){
        soundStream.stop();
        engineClose();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}


