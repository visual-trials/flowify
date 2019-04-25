<!doctype html>
<html lang="en">
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.1/css/bootstrap.min.css" integrity="sha384-WskhaSGFgHYWDcbwN70/dfYBj47jz9qbsMId/iRN3ewGhXQFZCSftd1LZCfmhktB" crossorigin="anonymous">
        <title>Flowify - survey questions</title>
    </head>
    <body>
        <br/>
        <h1><p class="text-center">Flowify - survey questions</p></h1>

        <div class="row">
            <div class="col-md-3">
            </div>
            <div class="col-md-6">
                <?php if ($_POST['have_submitted']): ?>
                
                    <?php 
                        $timestamp = time();
                        $random = rand();
                        file_put_contents("/home/surveys/{$timestamp}_{$random}.txt", print_r($_POST, true));
                    ?>
                    <br/>
                    <br/>
                    Thanks for filling in the survey questions!<br/>
                    <br/>
                    I really appreciate it.<br/>
                    <br/>
                    Regards,<br/>
                    <br/>
                    Jeffrey<br/>
                    <br/>
                    <br/>
                    <a href="/">Back to homepage</a><br/>
                    <br/>
                    <br/>
                <?php else: ?>
                    <br/>
                    <p class="text-center">Flowify is currently in early development. It would be very appriciated if you would like to fill in the survey questions below. This greatly helps me getting good feedback to make Flowify become useful.</p>
                    <form id="survey" action="/web/survey_questions.php" method="post">
                        <br/>
                        <h5>What is your impression about the <a href="/web/flowify.html">Flowify demo</a>? What did you like? What didn't you like?</h5>
                        <textarea name="impression" rows="5" cols="120" form="survey" ></textarea>
                        <br/>
                        <br/>
                        <h5>Do you think the Flowify demo shows any potential of becoming useful? What would be needed to make Flowify useful for you?</h5>
                        <textarea name="usefulness" rows="5" cols="120" form="survey" ></textarea>
                        <br/>
                        <br/>
                        <h5>Do you think Flowify can be made useful for educational purposes? In what form?</h5>
                        <textarea name="education" rows="5" cols="120" form="survey" ></textarea>
                        <br/>
                        <br/>
                        <h5>Do you think -in general- that visualization of code has any potential benefit? If so, what would that be? And for whom? If not, why not? What do you think is the biggest hurdle?</h5>
                        <textarea name="general_visualization" rows="5" cols="120" form="survey" ></textarea>
                        <br/>
                        <br/>
                        <h5>Can you think of a good usecase where to apply visualization of code? (eg. on github, in (or as an extension of) an IDE, for better code-reviewing, for communicating with others, for debugging)</h5>
                        <textarea name="aplication" rows="5" cols="120" form="survey" ></textarea>
                        <br/>
                        <br/>
                        <h5>Do you know of any other tools for visualization of code? Is so, what do you think of them? Do you use them? Is so, when? If not, why not?</h5>
                        <textarea name="knowlegde_visualization_tools" rows="5" cols="120" form="survey" ></textarea>
                        <br/>
                        <br/>
                        <h5>Are you acquainted with visual programming languages (VPL)? (as opposed to programming language visualization (PLV) tools, like Flowify) What is your opinion about them? Do or did you use them?</h5>
                        <textarea name="visual_programming" rows="5" cols="120" form="survey" ></textarea>
                        <br/>
                        <br/>
                        <h5>Do you have experience with programming yourself? Can you tell a little bit about your background?</h5>
                        <textarea name="programming_experience" rows="5" cols="120" form="survey" ></textarea>
                        <br/>
                        <br/>
                        <h5>Would you like updates or a reply from me? In that case, could you share your email-address with me? Thanks.</h5>
                        <textarea name="contact" rows="5" cols="120" form="survey" ></textarea>
                        <br/>
                        <br/>
                        <input type="hidden" name="have_submitted" value="1">
                        <input type="submit" value="Submit">
                        <br/>
                        <br/>
                    </form>
                <?php endif; ?>
            </div>
            <div class="col-md-3">
            </div>
        </div>
    </body>
</html>
